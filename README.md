# Trimui Model S – Development Environment

This repository provides a clean Docker-based cross-compilation environment for building ARMv5 applications for the Trimui Model S. It uses Ubuntu's gcc-arm-linux-gnueabi toolchain.

## Prerequisites

You need a `sysroot` directory containing the filesystem from your Trimui Model S device. See [Extracting the Sysroot](#extracting-the-sysroot) below.

## 1. Build the Docker Image

From the project root (where the Dockerfile is located):

```sh
podman build -t trimui-dev .
```

Or with Docker:

```sh
docker build -t trimui-dev .
```

## 2. Start the Development Container

Mount your working directory into `/src`:

```sh
podman run --rm -it -v "$PWD":/src trimui-dev
```

Inside the container, your project files will appear under `/src`.

## 3. Build the Example Program

Inside the container:

```sh
cd /src/examples/hellotrimui
arm-linux-gnueabi-gcc $CFLAGS --static hellotrimui.c $LDFLAGS -o hellotrimui
```

Or use the provided build script from the host:

```sh
cd examples/hellotrimui
./build.sh
```

This produces the binary:

```
hellotrimui
```

## 4. Run on the Trimui Model S

Copy the binary to your device (for example into `/mnt/SDCARD/APPS/hellotrimui/`) and launch it.

You should see a dark blue screen with "Hello Trimui" text centered in white, confirming the toolchain works.

## Extracting the Sysroot

Since we cannot publicly distribute the Trimui filesystem, you need to extract it from your own device.

**For detailed extraction instructions, see [SYSROOT_EXTRACTION.md](SYSROOT_EXTRACTION.md).**

### Quick Start: From an SD Card

1. Insert the Trimui Model S SD card into a Linux machine
2. Mount the root filesystem (usually the largest partition, often `/mnt/trimui` or similar)
3. Extract the filesystem:

```bash
mkdir -p sysroot
cp -r /mnt/trimui/* sysroot/
# Fix ownership if needed
sudo chown -R $(id -u):$(id -g) sysroot/
```

For more detailed instructions including SSH extraction, firmware images, and troubleshooting, see [SYSROOT_EXTRACTION.md](SYSROOT_EXTRACTION.md).

## Alternative Extraction Methods

- **SSH/Network**: If your device is accessible via SSH
- **Firmware Image**: Extract from official `.img` files
- **Tarball**: If you have a pre-packaged sysroot

See [SYSROOT_EXTRACTION.md](SYSROOT_EXTRACTION.md) for complete instructions.

## Project Structure

```
.
├── Dockerfile           # Build environment configuration
├── README.md           # This file
├── LICENSE             # MIT License
├── examples/
│   └── hellotrimui/
│       ├── hellotrimui.c       # Example framebuffer program
│       ├── font8x8_basic.h     # Built-in font data
│       └── build.sh            # Build script
└── sysroot/            # Extracted Trimui filesystem (not in git)
```

## Customization

### Adjusting Compiler Flags

Edit the `Dockerfile` to modify `CFLAGS` or `LDFLAGS` for different optimization levels or additional features.

### Using Different Toolchains

The Dockerfile uses Ubuntu's standard `gcc-arm-linux-gnueabi`. You can swap this for other ARM toolchains if needed.

## Troubleshooting

### "Cannot open sysroot"

Ensure the `sysroot` directory exists and contains the Trimui filesystem structure (should have `lib/`, `usr/`, `etc/`, etc.).

### Binary doesn't run on device

- Verify the binary is statically linked: `file hellotrimui` should show "ELF 32-bit LSB executable, ARM, EABI5"
- Check that your sysroot is from a compatible Trimui firmware version
- Ensure the device's framebuffer is at `/dev/fb0`

### Linker errors

Make sure your sysroot has the required libraries in `lib/` and `usr/lib/`. Some libraries may need to be compiled for your target architecture.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
