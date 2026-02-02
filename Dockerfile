FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# ------------------------------------------------------------
# 1. Install ARMv5 cross-compiler from Ubuntu repos
# ------------------------------------------------------------
RUN apt-get update && apt-get install -y \
    gcc-arm-linux-gnueabi \
    g++-arm-linux-gnueabi \
    build-essential \
    pkg-config \
    autoconf \
    automake \
    libtool \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# ------------------------------------------------------------
# 2. Set toolchain variables
# ------------------------------------------------------------
ENV CROSS=arm-linux-gnueabi
ENV CC="$CROSS-gcc"
ENV CXX="$CROSS-g++"

# ------------------------------------------------------------
# 3. Copy your sysroot into the container
# ------------------------------------------------------------
COPY sysroot /opt/sysroot
ENV SYSROOT=/opt/sysroot

# ------------------------------------------------------------
# 4. Export compiler flags (improved)
# ------------------------------------------------------------
ENV CFLAGS="--sysroot=$SYSROOT \
    -I$SYSROOT/usr/include \
    -I$SYSROOT/include \
    -march=armv5te \
    -mcpu=arm926ej-s \
    -msoft-float \
    -marm \
    -Os"

ENV LDFLAGS="--sysroot=$SYSROOT \
    -L$SYSROOT/usr/lib \
    -L$SYSROOT/lib"

# ------------------------------------------------------------
# 5. Create a working directory for your code
# ------------------------------------------------------------
WORKDIR /src
