# Sysroot Extraction Guide

This document explains how to extract the Trimui Model S sysroot from your own device for use with this development environment.

## What is a Sysroot?

A sysroot is a directory containing the essential filesystem structure (libraries, headers, etc.) from the target device. It's used by the cross-compiler to link applications against the correct libraries for your device architecture.

## Prerequisites

- A Trimui Model S device
- Access to the device's filesystem (via SD card or SSH)
- A Linux machine for extraction

## Method 1: Extract from SD Card (Recommended)

This is the most straightforward method if you have direct access to the SD card.

### Steps:

1. **Insert SD card into your Linux machine**
   - Identify the card:
     ```bash
     lsblk | grep -E "mmcblk|sdb|sdc"
     ```
   - Look for a device around 8-32 GB in size

2. **Create mount points**
   ```bash
   mkdir -p /mnt/trimui_root
   mkdir -p ~/trimui-sysroot
   ```

3. **Mount the root partition**
   - The SD card typically has multiple partitions. The root filesystem is usually the largest one.
   - If the card is `/dev/mmcblk0`, the root partition might be `/dev/mmcblk0p3` or similar:
     ```bash
     sudo mount /dev/mmcblk0p3 /mnt/trimui_root
     ```
   - If it's `/dev/sdb`, try `/dev/sdb1`, `/dev/sdb2`, etc.

4. **Extract the filesystem**
   ```bash
   cp -r /mnt/trimui_root/* ~/trimui-sysroot/
   
   # Fix ownership
   sudo chown -R $(id -u):$(id -g) ~/trimui-sysroot
   ```

5. **Verify the extraction**
   ```bash
   ls ~/trimui-sysroot/
   # Should show: bin, boot, dev, etc, home, lib, mnt, proc, root, run, sbin, srv, sys, tmp, usr, var
   ```

6. **Copy to project**
   ```bash
   cp -r ~/trimui-sysroot /path/to/trimui-model-s-devkit/sysroot
   ```

7. **Unmount the card**
   ```bash
   sudo umount /mnt/trimui_root
   ```

## Method 2: Extract via SSH/Network

If your Trimui device is accessible via SSH:

### Steps:

1. **Connect to device and find IP address**
   ```bash
   # On the Trimui device or via SSH:
   ip addr show
   ```

2. **Extract filesystem via SSH (from your Linux machine)**
   ```bash
   # Create sysroot directory
   mkdir -p trimui-sysroot
   
   # Use rsync (recommended for efficiency)
   rsync -av --delete \
     --exclude=/proc --exclude=/sys --exclude=/dev \
     root@TRIMUI_IP:/ trimui-sysroot/
   ```
   
   Replace `TRIMUI_IP` with your device's actual IP address.

3. **Fix permissions**
   ```bash
   # If rsync ran as root, fix ownership
   sudo chown -R $(id -u):$(id -g) trimui-sysroot
   ```

4. **Copy to project**
   ```bash
   cp -r trimui-sysroot /path/to/trimui-model-s-devkit/sysroot
   ```

## Method 3: Extract from Firmware Image

If you have the official Trimui firmware as an `.img` file:

### Steps:

1. **Find the filesystem image**
   - Extract or locate the `.img` file from firmware archives

2. **Mount the image**
   ```bash
   # Create loop device
   sudo mount -o loop firmware.img /mnt/trimui_fw
   ```

3. **Extract filesystem**
   ```bash
   mkdir -p ~/trimui-sysroot
   cp -r /mnt/trimui_fw/* ~/trimui-sysroot/
   
   # Fix ownership
   sudo chown -R $(id -u):$(id -g) ~/trimui-sysroot
   ```

4. **Unmount**
   ```bash
   sudo umount /mnt/trimui_fw
   ```

5. **Copy to project**
   ```bash
   cp -r ~/trimui-sysroot /path/to/trimui-model-s-devkit/sysroot
   ```

## Method 4: Using tar Archive

If the sysroot is available as a tarball:

```bash
mkdir -p ~/trimui-sysroot
tar -xzf trimui-sysroot.tar.gz -C ~/trimui-sysroot/

# Fix permissions if extracted as root
sudo chown -R $(id -u):$(id -g) ~/trimui-sysroot

# Copy to project
cp -r ~/trimui-sysroot /path/to/trimui-model-s-devkit/sysroot
```

## Verifying Your Sysroot

After extraction, verify that essential directories exist:

```bash
ls -la sysroot/
```

Should contain:
- `lib/` - Runtime libraries
- `usr/lib/` - Additional libraries
- `usr/include/` - Header files
- `etc/` - Configuration files
- `bin/`, `sbin/` - Executables

Critical files to check:
```bash
ls -la sysroot/lib/libc.so.6         # C library
ls -la sysroot/lib/libm.so.6         # Math library
ls -la sysroot/usr/lib/              # Development libraries
```

## Troubleshooting

### "Permission denied" errors during extraction

Run the copy/rsync commands with `sudo`:
```bash
sudo cp -r /mnt/trimui_root/* trimui-sysroot/
sudo chown -R $(id -u):$(id -g) trimui-sysroot
```

### "No such file or directory" when mounting

Check the correct partition:
```bash
sudo fdisk -l /dev/mmcblk0
# or
sudo fdisk -l /dev/sdb
```

Look for the largest partition (usually marked as Linux).

### Symlinks pointing to wrong paths

This is normal. The cross-compiler handles relative symlinks correctly.

### Compilation fails with "library not found"

Your sysroot might be incomplete. Verify:
1. Correct device sysroot was extracted
2. All directories exist: `lib/`, `usr/lib/`, `usr/include/`
3. Run `file sysroot/lib/libc.so.6` to confirm it's for ARM

## Size Expectations

A typical Trimui sysroot is **500 MB - 2 GB** depending on included software.

## Keep Your Sysroot Updated

If you update your Trimui device firmware, re-extract the sysroot to ensure compatibility.

## Security Note

The sysroot contains sensitive files from your device. Keep it private and don't share publicly.
