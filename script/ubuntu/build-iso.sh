#!/bin/bash
set -e

# get parameters
ROOT_PASSWORD=${1:-"`pwgen -N1 -B`"}
TARGET_ISO=${2:-"`pwd`/ubuntu-16.04.3-server-amd64-unattended-$ROOT_PASSWORD.iso"}

# encrypt root password
ROOT_PASSWORD_ENCRYPTED="`printf "$ROOT_PASSWORD" | mkpasswd -s -m sha-512`"

# get directories
CURRENT_DIR="`pwd`"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TMP_DOWNLOAD_DIR="`mktemp -d`"
TMP_DISC_DIR="`mktemp -d`"
TMP_INITRD_DIR="`mktemp -d`"

# extract netboot iso
7z x "/home/dan/Documents/ubuntu-16.04.3-server-amd64.iso" "-o$TMP_DISC_DIR"

# patch boot menu
echo $SCRIPT_DIR
cd "$TMP_DISC_DIR"
patch -p1 -i "$SCRIPT_DIR/boot-menu.patch"

# prepare preseed.cfg
ROOT_PASSWORD_ENCRYPTED_SAFE=$(printf '%s\n' "$ROOT_PASSWORD_ENCRYPTED" | sed 's/[[\.*/]/\\&/g; s/$$/\\&/; s/^^/\\&/')
cp "$SCRIPT_DIR/preseed.cfg" "$TMP_INITRD_DIR/preseed.cfg"
sed -i "s/d-i passwd\\/root-password-crypted password.*/d-i passwd\\/root-password-crypted password $ROOT_PASSWORD_ENCRYPTED_SAFE/g" "$TMP_INITRD_DIR/preseed.cfg"

# append preseed.cfg to initrd
cd "$TMP_INITRD_DIR"
cat "$TMP_DISC_DIR/install/initrd.gz" | gzip -d > "$TMP_INITRD_DIR/initrd"
echo "preseed.cfg" | cpio -o -H newc -A -F "$TMP_INITRD_DIR/initrd"
cat "$TMP_INITRD_DIR/initrd" | gzip -9c > "$TMP_DISC_DIR/install/initrd.gz"

cp "$TMP_INITRD_DIR/preseed.cfg" "$TMP_DISC_DIR/preseed/preseed.cfg"

# build iso
cd "$TMP_DISC_DIR"
rm -r '[BOOT]'
mkisofs -U -V "ubuntu 16.04 unattended" -J -joliet-long -r -v -T -o "$TARGET_ISO" -b boot/grub/efi.img -no-emul-boot ./

# go back to initial directory
cd "$CURRENT_DIR"

# delete all temporary directories
rm -r "$TMP_DOWNLOAD_DIR"
rm -r "$TMP_DISC_DIR"
rm -r "$TMP_INITRD_DIR"

# done
echo "The 'root' password is '$ROOT_PASSWORD'. Next steps: install system, login via root, change root password"
