ARG UBUNTU_VERSION=22.04
FROM ubuntu:${UBUNTU_VERSION}
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
    python3 xvfb xauth libgl1 libegl1 libgles2 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /tmp
