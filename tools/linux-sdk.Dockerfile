FROM ubuntu:24.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
    ca-certificates cmake ninja-build g++ python3 libsdl2-dev libsdl2-ttf-dev libsfml-dev \
    libegl1 libgles2 qt6-base-dev libxtst-dev xvfb xauth xdotool python3-tk python3-pygame python3-venv \
    python3-pil python3-pil.imagetk clang-format-16 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
