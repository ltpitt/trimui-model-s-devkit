# Sysroot Extraction Guide

This document explains how to extract the Trimui Model S sysroot from your own device for use with this development environment.

> **Note**: This guide was written and tested on **Linux Mint Cinnamon 22**. Commands may vary slightly on other Linux distributions.

## What is a Sysroot?

A sysroot is a directory containing the essential filesystem structure (libraries, headers, etc.) from the target device. It's used by the cross-compiler to link applications against the correct libraries for your device architecture.

## Prerequisites

- A Trimui Model S device
- Access to the device's filesystem (via SD card, ADB, or device terminal)
- A Linux machine for extraction

## Important Notes

- **FAT32 SD Card**: The Trimui Model S uses FAT32, which cannot preserve Linux symlinks, permissions, or special files. **Tar archives are essential** to avoid corrupting the sysroot.
- **Modular Archives**: The recommended approach creates separate `lib.tar` and `usrlib.tar` archives containing only the essential libraries and headers needed for cross-compilation.
- **Two Primary Methods**: Use either device terminal or ADB (Android Debug Bridge) to create the archives on the device.
## Method 1: Extract via Device Terminal (Recommended)

If your Trimui has a built-in terminal, this is the simplest approach.

### Steps:

1. **Create tar archives on the device terminal**
   ```bash
   mkdir -p /mnt/SDCARD/sysroot
   
   # Create lib archive (core system libraries)
   cd /lib && tar -czf /mnt/SDCARD/sysroot/lib.tar .
   
   # Create usr/lib archive (development libraries)
   cd /usr/lib && tar -czf /mnt/SDCARD/sysroot/usrlib.tar .
   ```

2. **Remove SD card and mount on your Linux machine**
   ```bash
   # Find the SD card
   lsblk | grep -E "mmcblk|sdb|sdc"
   
   # Mount the largest partition (usually /dev/mmcblk0p3 or similar)
   sudo mount /dev/mmcblk0p3 /mnt/trimui_sd
   ```

3. **Copy archives to your machine**
   ```bash
   cp /mnt/trimui_sd/sysroot/lib.tar .
   cp /mnt/trimui_sd/sysroot/usrlib.tar .
   
   sudo umount /mnt/trimui_sd
   ```

4. **Extract the archives**
   ```bash
   mkdir -p sysroot/lib sysroot/usr/lib
   
   tar -xzf lib.tar -C sysroot/lib/
   tar -xzf usrlib.tar -C sysroot/usr/
   ```

5. **Copy to project**
   ```bash
   cp -r sysroot /path/to/trimui-model-s-devkit/
   ```

## Method 2: Extract via ADB (Android Debug Bridge)

If your Trimui device supports ADB, this is an alternative to the terminal method.

### Prerequisites

```bash
# Install ADB on Linux Mint/Ubuntu
sudo apt install android-tools-adb
```

### Steps:

1. **Connect device via USB**
   ```bash
   adb devices  # Verify device is detected
   ```

2. **Create tar archives on the device**
   ```bash
   adb shell "mkdir -p /mnt/SDCARD/sysroot"
   adb shell "cd /lib && tar -czf /mnt/SDCARD/sysroot/lib.tar ."
   adb shell "cd /usr && tar -czf /mnt/SDCARD/sysroot/usrlib.tar lib"
   ```

3. **Pull archives to your machine**
   ```bash
   adb pull /mnt/SDCARD/sysroot/lib.tar .
   adb pull /mnt/SDCARD/sysroot/usrlib.tar .
   ```

4. **Extract the archives**
   ```bash
   mkdir -p sysroot/lib sysroot/usr/lib sysroot/usr/include
   
   tar -xzf lib.tar -C sysroot/lib/
   tar -xzf usrlib.tar -C sysroot/usr/
   ```

5. **Copy to project**
   ```bash
   cp -r sysroot /path/to/trimui-model-s-devkit/
   ```

## Verifying Your Sysroot

After extraction, verify the essential libraries exist:

```bash
ls -la sysroot/
ls -la sysroot/lib/
ls -la sysroot/usr/lib/
```

Should contain:
- `lib/libc.so.6` - C library (critical)
- `lib/libm.so.6` - Math library
- `usr/lib/` - Development libraries

### Quick validation:
```bash
file sysroot/lib/libc.so.6  # Should show "ELF 32-bit LSB shared object, ARM"
```

## Troubleshooting

### Tar extraction fails

```bash
# Check tar file integrity
tar -tzf lib.tar | head  # Should list files without errors
```

### SD card mount fails

```bash
# List available devices and partitions
lsblk
sudo fdisk -l

# Try different partition (usually p1, p2, or p3)
sudo mount /dev/mmcblk0p2 /mnt/trimui_sd
```

### Symlinks not preserved

This indicates the tar was created incorrectly on the device. Recreate it with:
```bash
cd /lib && tar -czf /mnt/SDCARD/sysroot/lib.tar --exclude="lost+found" .
```

### Compilation fails with "library not found"

Verify the sysroot structure:
```bash
file sysroot/lib/libc.so.6      # Should show "ELF 32-bit LSB shared object"
ls sysroot/usr/lib/             # Should have libraries
find sysroot/ -type l | head    # Check for symlinks (should exist)
```

### ADB device not found

```bash
adb kill-server
adb start-server
adb devices  # Verify USB debugging is enabled on device
```

## Size Expectations

A typical Trimui sysroot is **500 MB - 2 GB** depending on included software.

## Keep Your Sysroot Updated

If you update your Trimui device firmware, re-extract the sysroot to ensure compatibility.

## Security Note

The sysroot contains sensitive files from your device. Keep it private and don't share publicly.
