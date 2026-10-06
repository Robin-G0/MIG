#include "camera.hpp"
#include "pixels.hpp"
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace mig::native {
std::int64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}
namespace {
int camera_ioctl(int fd, unsigned long request, void* arg) {
    int result;
    do {
        result = ioctl(fd, request, arg);
    } while (result < 0 && errno == EINTR);
    return result;
}
void require(bool success, const char* operation) {
    if (!success) {
        throw std::runtime_error(std::string(operation) + ": " + std::strerror(errno));
    }
}
} // namespace
struct Camera::Impl {
    int fd{-1};
    unsigned width{}, height{}, stride{};
    struct Buffer {
        void* data{MAP_FAILED};
        std::size_t size{};
    };
    std::vector<Buffer> buffers;
    std::atomic<bool> stopped{};
    bool streaming{};
    ~Impl() {
        if (streaming) {
            auto type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            camera_ioctl(fd, VIDIOC_STREAMOFF, &type);
        }
        for (const auto& b : buffers) {
            if (b.data != MAP_FAILED) {
                munmap(b.data, b.size);
            }
        }
        if (fd >= 0) {
            close(fd);
        }
    }
};
Camera::Camera(unsigned index) : impl_(std::make_unique<Impl>()) {
    impl_->fd = open(("/dev/video" + std::to_string(index)).c_str(), O_RDWR | O_NONBLOCK);
    require(impl_->fd >= 0, "Open V4L2 camera");
    v4l2_capability caps{};
    require(camera_ioctl(impl_->fd, VIDIOC_QUERYCAP, &caps) == 0, "Query V4L2 camera");
    const auto capabilities =
        caps.capabilities & V4L2_CAP_DEVICE_CAPS ? caps.device_caps : caps.capabilities;
    if (!(capabilities & V4L2_CAP_VIDEO_CAPTURE) || !(capabilities & V4L2_CAP_STREAMING)) {
        throw std::runtime_error("Camera must support V4L2 single-plane streaming capture");
    }
    v4l2_format format{};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = 1280;
    format.fmt.pix.height = 720;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    format.fmt.pix.field = V4L2_FIELD_ANY;
    require(camera_ioctl(impl_->fd, VIDIOC_S_FMT, &format) == 0, "Set YUYV camera format");
    const auto& pixels = format.fmt.pix;
    if (pixels.pixelformat != V4L2_PIX_FMT_YUYV || !pixels.width || !pixels.height ||
        pixels.width > 4096 || pixels.height > 4096 || pixels.width % 2 ||
        pixels.bytesperline < pixels.width * 2) {
        throw std::runtime_error(
            "Unsupported camera format: requires even-width YUYV <=4096 pixels");
    }
    impl_->width = pixels.width;
    impl_->height = pixels.height;
    impl_->stride = pixels.bytesperline;
    v4l2_requestbuffers request{};
    request.count = 4;
    request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    request.memory = V4L2_MEMORY_MMAP;
    require(camera_ioctl(impl_->fd, VIDIOC_REQBUFS, &request) == 0 && request.count > 0,
            "Allocate camera buffers");
    impl_->buffers.resize(request.count);
    for (unsigned i = 0; i < request.count; ++i) {
        v4l2_buffer b{};
        b.type = request.type;
        b.memory = request.memory;
        b.index = i;
        require(camera_ioctl(impl_->fd, VIDIOC_QUERYBUF, &b) == 0, "Query camera buffer");
        impl_->buffers[i] = {
            mmap(nullptr, b.length, PROT_READ | PROT_WRITE, MAP_SHARED, impl_->fd, b.m.offset),
            b.length};
        require(impl_->buffers[i].data != MAP_FAILED, "Map camera buffer");
        require(camera_ioctl(impl_->fd, VIDIOC_QBUF, &b) == 0, "Queue camera buffer");
    }
    auto type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    require(camera_ioctl(impl_->fd, VIDIOC_STREAMON, &type) == 0, "Start camera");
    impl_->streaming = true;
}
Camera::~Camera() = default;
void Camera::shutdown() noexcept {
    impl_->stopped = true;
}
bool Camera::read(VideoFrame& frame) {
    return read(frame, true);
}
bool Camera::read(VideoFrame& frame, bool include_display_buffer) {
    if (impl_->stopped) {
        return false;
    }
    pollfd descriptor{impl_->fd, POLLIN, 0};
    const auto ready = poll(&descriptor, 1, 100);
    if (ready == 0 || (ready < 0 && errno == EINTR) || impl_->stopped) {
        return false;
    }
    require(ready > 0, "Wait for camera");
    v4l2_buffer b{};
    b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    b.memory = V4L2_MEMORY_MMAP;
    if (camera_ioctl(impl_->fd, VIDIOC_DQBUF, &b) < 0) {
        if (errno == EAGAIN) {
            return false;
        }
        require(false, "Read camera buffer");
    }
    struct Requeue {
        int fd;
        v4l2_buffer& buffer;
        ~Requeue() {
            camera_ioctl(fd, VIDIOC_QBUF, &buffer);
        }
    } guard{impl_->fd, b};
    if (b.index >= impl_->buffers.size() || b.bytesused > impl_->buffers[b.index].size ||
        b.bytesused < std::size_t(impl_->height - 1) * impl_->stride + impl_->width * 2) {
        throw std::runtime_error("Truncated camera buffer");
    }
    frame.width = int(impl_->width);
    frame.height = int(impl_->height);
    frame.capture_ms = (b.flags & V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC)
                           ? b.timestamp.tv_sec * std::int64_t(1000) + b.timestamp.tv_usec / 1000
                           : now_ms();
    const auto* data = static_cast<const std::uint8_t*>(impl_->buffers[b.index].data);
    const std::span<const std::uint8_t> source(data, b.bytesused);
    if (include_display_buffer) {
        convert_yuyv<true>(source, impl_->stride, frame);
    } else {
        convert_yuyv<false>(source, impl_->stride, frame);
    }
    return true;
}
} // namespace mig::native
