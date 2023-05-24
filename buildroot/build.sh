#!/bin/sh

# Configuration
. ./config.sh

# Convenience definitions
BR=buildroot-${BR_VERSION}

if [ ! -f buildroot-${BR_VERSION}.tar.bz2 ]; then
	echo ">> Fetching buildroot-${BR_VERSION}.tar.bz2"
	${WGET} http://buildroot.uclibc.org/downloads/buildroot-${BR_VERSION}.tar.bz2
	if [ $? -ne 0 ]; then
		${RM} -f buildroot-${BR_VERSION}.tar.bz2
		echo "*** Download failed"
		exit 1
	fi
fi

if [ ! -d ${BR} ]; then
	echo ">> Extracting buildroot-${BR_VERSION}.tar.bz2"
	${TAR} xf buildroot-${BR_VERSION}.tar.bz2
	if [ $? -ne 0 ]; then
		${RM} -f ${BR}
		echo "*** Failed"
		exit 1
	fi
fi

if [ ! -f ${BR}/configs/${BR_COMPANY}_${BR_PROJECT}_defconfig ]; then
	echo ">> Patching buildroot tree"
	(cd ${BR} && ${PATCH} -p1 < ../buildroot-${BR_VERSION}.diff)
	if [ $? -ne 0 ]; then
		echo "*** Failed"
		exit 1
	fi
fi

if [ ! -f ${BR}/.config ]; then
	echo ">> Activating default configuration"
	(cd ${BR} && ${MAKE} ${BR_COMPANY}_${BR_PROJECT}_defconfig)
	if [ $? -ne 0 ]; then
		echo "*** Failed"
		exit 1
	fi
fi

if [ ! -f ${BR}/output/images/rootfs.tar ]; then
	echo ">> Building"
	(cd ${BR} && ${MAKE})
	if [ $? -ne 0 ]; then
		echo "*** Failed"
		exit 1
	fi
fi
