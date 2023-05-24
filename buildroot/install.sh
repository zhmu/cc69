#!/bin/sh

# Configuration
. ./config.sh

DEVICE=sdd
D=/mnt

# Convenience definitions
BR=buildroot-${BR_VERSION}

# Sanity checks
if [ ! -f ${BR}/output/images/rootfs.tar ]; then
	echo ">> Image not build; did you run build.sh ?"
	exit 1
fi

${GREP} -q " ${D} " /proc/mounts
if [ $? -ne 1 ]; then
	echo ">> Temporary mount point ${D} is already in use, aborting"
	exit 1
fi

${GREP} -q /dev/${DEVICE} /proc/mounts
if [ $? -ne 1 ]; then
	echo ">> Destination device ${DEVICE} still has filesystems mounted on it, aborting"
	exit 1
fi

if [ `${CAT} /sys/block/${DEVICE}/removable` -ne 1 ]; then
	echo ">> Destination device ${DEVICE} isn't marked as removable, aborting"
	exit 1
fi

if [ `${ID} -u` -ne 0 ]; then
	echo ">> You are not root, aborting"
	exit 1
fi

# Final way out
echo -n ">> WARNING: This script is about to remove all data from /dev/${DEVICE} ("
tr -d '\n' < /sys/block/${DEVICE}/device/model
echo -n ") - do you want to continue (y/n)? "
#read q
q=y
if [ "$q" != "y" -a "$q" != "Y" ]; then
	echo "Aborting"
	exit 1
fi

echo ">> Re-partitioning device..."
# Blast the partition table; this is easier than figuring out what we have to delete
${DD} if=/dev/zero of=/dev/${DEVICE} bs=1k count=1
# One partition, complete disk, active
echo "n\n\n\n\n\na\n1\nw\nq\n" | ${FDISK} /dev/${DEVICE}
if [ $? -ne 0 ]; then
	echo "Failure"
	exit 1
fi

echo ">> Creating filesystem..."
mkfs.ext2 /dev/${DEVICE}1
if [ $? -ne 0 ]; then
	echo "Failure"
	exit 1
fi
tune2fs -c 0 -i 0 /dev/${DEVICE}1
if [ $? -ne 0 ]; then
	echo "Failure"
	exit 1
fi

echo ">> Mounting filesystem..."
${MOUNT} /dev/${DEVICE}1 ${D}
if [ $? -ne 0 ]; then
	echo "Failure"
	exit 1
fi

echo ">> Installing bootloader..."
${GRUB_INSTALL} --no-floppy --boot-directory=${D}/boot /dev/${DEVICE}
if [ $? -ne 0 ]; then
	echo "Failure"
	exit 1
fi

${CAT} >${D}/boot/grub/grub.cfg <<GRUBCFG
set default="0"
set timeout=0

menuentry "Linux" --class gnu-linux --class os {
        insmod  part_msdos
        insmod  ext2
	linux   /boot/bzImage console=ttyS0,115200 console=tty root=/dev/sda1 i915.modeset=1 ro
}
GRUBCFG
if [ $? -ne 0 ]; then
	echo "Failure writing grub.cfg"
	exit 1
fi

echo ">> Extracting image..."
${TAR} xf ${BR}/output/images/rootfs.tar -C ${D}
if [ $? -ne 0 ]; then
	echo "Failure"
	exit 1
fi

echo ">> Configuring X..."
# Patch the inittab so that X will be launched during bootup
echo "::once:/bin/su - -- root -l -c '/usr/bin/startx'" >> ${D}/etc/inittab
if [ $? -ne 0 ]; then
	echo "Failure"
	exit 1
fi

# Create an xinitrc run by X
${CAT} > ${D}/usr/lib/X11/xinit/xinitrc <<XINITRC
#!/bin/sh
while true; do
	/usr/bin/xterm
done
XINITRC

# Symlink root's directory so that X can write its .xsession-errors there
${RM} -rf ${D}/root
${LN} -sf tmp ${D}/root

echo ">> Dismounting..."
${UMOUNT} ${D}
${SYNC}
