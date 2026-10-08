FROM ubuntu:22.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
    ca-certificates git ninja-build g++ ccache python3 python3-pip python3-venv \
    qt6-base-dev libxtst-dev libegl1 libgles2 libsdl2-dev libsdl2-ttf-dev libsfml-dev \
    xvfb xauth xdotool g++-aarch64-linux-gnu qemu-user dpkg-dev file apt-utils gnupg \
    python3-tk python3-pil python3-pil.imagetk python3-pygame libpython3.10 \
    && python3 -m pip install --no-cache-dir cmake==3.31.10 build twine auditwheel patchelf pyinstaller==6.16.0 pillow==11.3.0 pygame==2.6.1 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
