/*
 * Copyright 2007 Peter Hutterer
 * Copyright 2009 Przemysław Firszt
 * (Originally from the x86-input-random example device)
 * Copyright 2012 Rink Springer
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Red Hat
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.  Red
 * Hat makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <linux/input.h>
#include <linux/types.h>

#include <xf86_OSproc.h>

#include <unistd.h>

#include <xf86.h>
#include <xf86Xinput.h>
#include <exevents.h>
#include <xorgVersion.h>
#include <xkbsrv.h>

#define REOPEN_ATTEMPTS 10


#ifdef HAVE_PROPERTIES
#include <xserver-properties.h>
/* 1.6 has properties, but no labels */
#ifdef AXIS_LABEL_PROP
#define HAVE_LABELS
#else
#undef HAVE_LABELS
#endif

#endif


#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <xorg-server.h>
#include <xorgVersion.h>
#include <xf86Module.h>
#include <X11/Xatom.h>

#include "irtouch.h"

static InputInfoPtr IrtouchPreInit(InputDriverPtr drv, IDevPtr dev, int flags);
static void IrtouchUnInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags);
static pointer IrtouchPlug(pointer module, pointer options, int *errmaj, int  *errmin);
static void IrtouchUnplug(pointer p);
static void IrtouchReadInput(InputInfoPtr pInfo);
static int IrtouchControl(DeviceIntPtr	device,int what);
static int _irtouch_init_buttons(DeviceIntPtr device);
static int _irtouch_init_axes(DeviceIntPtr device);
static void IrtouchDeviceOn(DeviceIntPtr device);

#define MAX_RANGE 64000
int touch_min_x = 0;
int touch_max_x = 63087;
int touch_min_y = 0;
int touch_max_y = 64015;

_X_EXPORT InputDriverRec RANDOM = {
	1,
	"irtouch",
	NULL,
	IrtouchPreInit,
	IrtouchUnInit,
	NULL,
	0
};

static XF86ModuleVersionInfo IrtouchVersionRec = {
	"irtouch",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR,
	PACKAGE_VERSION_PATCHLEVEL,
	ABI_CLASS_XINPUT,
	ABI_XINPUT_VERSION,
	MOD_CLASS_XINPUT,
	{0, 0, 0, 0}
};

_X_EXPORT XF86ModuleData irtouchModuleData = {
	&IrtouchVersionRec,
	&IrtouchPlug,
	&IrtouchUnplug
};

static void
IrtouchUnplug(pointer p)
{
}

static CARD32
IrtouchReopenTimer(OsTimerPtr timer, CARD32 time, pointer arg)
{
	InputInfoPtr pInfo = (InputInfoPtr)arg;
	IrtouchDevicePtr pIrtouch = pInfo->private;

	do {
		pInfo->fd = open(pIrtouch->device, O_RDONLY);
	} while(pInfo->fd < 0 && errno == EINTR);

	if (pInfo->fd >= 0) {
		// Reopening worked
		xf86Msg(X_INFO, "%s: Reopen ok, onning device\n", pInfo->name);
		pIrtouch->reopen_left = 0;
		IrtouchDeviceOn(pInfo->dev);
		return 0;
	}

	if (--pIrtouch->reopen_left > 0)
		return 100; /* come back in 100ms */

	xf86Msg(X_ERROR, "%s: Cannot open device '%s', giving up.\n",
	 pInfo->name, pIrtouch->device);
	return 0;
}

static pointer
IrtouchPlug(pointer module, pointer options, int* errmaj, int* errmin)
{
	xf86AddInputDriver(&RANDOM, module, 0);
	return module;
}

static InputInfoPtr IrtouchPreInit(InputDriverPtr drv, IDevPtr dev, int flags)
{
	InputInfoPtr		pInfo;
	IrtouchDevicePtr	pIrtouch;

	pInfo = xf86AllocateInput(drv, 0);
	if (pInfo == NULL)
		return NULL;

	pIrtouch = xcalloc(1, sizeof(IrtouchDeviceRec));
	if (pIrtouch == NULL) {
		pInfo->private = NULL;
		xf86DeleteInput(pInfo, 0);
		return NULL;
	}

	pInfo->private = pIrtouch;
	pInfo->name = xstrdup(dev->identifier);
	pInfo->flags = 0;
	pInfo->history_size = 0;
	pInfo->type_name = XI_TOUCHSCREEN; /* see XI.h */
	pInfo->conf_idev = dev;
	pInfo->read_input = IrtouchReadInput; /* new data avl */
	pInfo->switch_mode = NULL; /* toggle absolute/relative mode */
	pInfo->device_control = IrtouchControl; /* enable/disable dev */
	pInfo->always_core_feedback = NULL;
	/* process driver specific options */
	pIrtouch->device = xf86SetStrOption(dev->commonOptions, "Device", NULL);
	if (pIrtouch->device == NULL) {
		xf86Msg(X_ERROR, "%s: No device specified.\n", pInfo->name);
		goto fail;
	}

	xf86Msg(X_INFO, "%s: Using device %s.\n", pInfo->name, pIrtouch->device);

	/* process generic options */
	xf86CollectInputOptions(pInfo, NULL, NULL);
	xf86ProcessCommonOptions(pInfo, pInfo->options);

	/* fetch coordinate ranges */
	pIrtouch->min_x = xf86SetIntOption(pInfo->options, "MinX", 0);
	pIrtouch->min_y = xf86SetIntOption(pInfo->options, "MinY", 0);
	pIrtouch->max_x = xf86SetIntOption(pInfo->options, "MaxX", 65535);
	pIrtouch->max_y = xf86SetIntOption(pInfo->options, "MaxY", 65535);
	xf86Msg(X_INFO, "%s: Calibration coordinates: X=[%d..%d], Y=[%d..%d].\n",
	 pInfo->name, pIrtouch->min_x, pIrtouch->max_x, pIrtouch->min_y, pIrtouch->max_y);

	/* reopen the device when it's needed */
	pIrtouch->reopen_timer = NULL;
	pInfo->fd = -1;

	/* do more funky stuff */
	xf86Msg(X_INFO, "%s: Init OK\n", pInfo->name);
	pInfo->flags |= XI86_POINTER_CAPABLE;
	pInfo->flags |= XI86_ALWAYS_CORE;
	pInfo->flags |= XI86_CORE_POINTER;
	pInfo->flags |= XI86_CONFIGURED;
	return pInfo;

fail:
	pInfo->private = NULL;
	xfree(pIrtouch);
	xf86DeleteInput(pInfo, 0);
	return NULL;
}

static void IrtouchUnInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags)
{
	IrtouchDevicePtr pIrtouch = pInfo->private;
	xf86Msg(X_INFO, "%s: Uninit\n", pInfo->name);
	if (pIrtouch->device != NULL) {
		xfree(pIrtouch->device);
		pIrtouch->device = NULL;
		/* Common error - pInfo->private must be NULL or valid memoy before
		 * passing into xf86DeleteInput */
		pInfo->private = NULL;
	}
	xf86DeleteInput(pInfo, 0);
}


static int
_irtouch_init_buttons(DeviceIntPtr device)
{
	InputInfoPtr pInfo = device->public.devicePrivate;
	CARD8* map;
	Atom* labels;
	int i;
	int ret = Success;
	const int num_buttons = 2;

	map = xcalloc(num_buttons, sizeof(CARD8));

	for (i = 0; i < num_buttons; i++)
		map[i] = i;

	labels = xalloc(num_buttons * sizeof(Atom));

	if (!InitButtonClassDeviceStruct(device, num_buttons,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
	                                 labels,
#endif
	                                 map)) {
		xf86Msg(X_ERROR, "%s: Failed to register buttons.\n", pInfo->name);
		ret = BadAlloc;
	}

	xfree(labels);
	xfree(map);
	return ret;
}

static void IrtouchInitAxesLabels(IrtouchDevicePtr pIrtouch, int natoms, Atom *atoms)
{
#ifdef HAVE_LABELS
	Atom atom;
	int axis;
	char **labels;
	int labels_len = 0;
	char *misc_label;

	labels	 = rel_labels;
	labels_len = ArrayLength(rel_labels);
	misc_label = AXIS_LABEL_PROP_REL_MISC;
	xf86Msg(X_INFO, "%s: IrtouchInitAxesLabels\n", pInfo->name);

	memset(atoms, 0, natoms * sizeof(Atom));

	/* Now fill the ones we know */
	for (axis = 0; axis < labels_len; axis++) {
		if (pIrtouch->axis_map[axis] == -1)
			continue;

		atom = XIGetKnownProperty(labels[axis]);
		if (atom == NULL) /* Should not happen */
			continue;

		atoms[pIrtouch->axis_map[axis]] = atom;
	}
#endif
}

static int
_irtouch_init_axes(DeviceIntPtr device)
{
	InputInfoPtr pInfo = device->public.devicePrivate;
	IrtouchDevicePtr pIrtouch = pInfo->private;
	Atom* atoms;
	int num_axes = 2;
	atoms = xalloc(num_axes * sizeof(Atom));

	IrtouchInitAxesLabels(pIrtouch, num_axes, atoms);
	if (!InitValuatorClassDeviceStruct(device,
	                                   2,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
	                                   atoms,
#endif
	                                   GetMotionHistorySize(),
	                                   Absolute))
		return BadAlloc;

	pInfo->dev->valuator->mode = Absolute;
	if (!InitAbsoluteClassDeviceStruct(device))
		return BadAlloc;

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
# define SET_AXIS(i) \
	 xf86InitValuatorAxisStruct(device, i, atoms[i], 0, MAX_RANGE, MAX_RANGE, 0, MAX_RANGE)
#else
# define SET_AXIS(i) \
	 xf86InitValuatorAxisStruct(device, i, 0, MAX_RANGE, MAX_RANGE, 0, MAX_RANGE)
#endif

	SET_AXIS(0);
	SET_AXIS(1);
	xf86InitValuatorDefaults(device, 0);
	xf86InitValuatorDefaults(device, 1);
	xfree(atoms);
	return Success;
}

static void
IrtouchDeviceOn(DeviceIntPtr device)
{
	InputInfoPtr pInfo = device->public.devicePrivate;
	IrtouchDevicePtr pIrtouch = pInfo->private;

	xf86Msg(X_INFO, "%s: fd=%d\n", pInfo->name, pInfo->fd);
	if (pInfo->fd < 0) {
		xf86Msg(X_INFO, "%s: No device, opening\n", pInfo->name);
		pIrtouch->reopen_left = REOPEN_ATTEMPTS;
		pIrtouch->reopen_timer = TimerSet(pIrtouch->reopen_timer, 0, 100, IrtouchReopenTimer, pInfo);
		xf86Msg(X_INFO, "%s: No timer, setting was %p\n", pInfo->name, pIrtouch->reopen_timer);
	} else {
		xf86Msg(X_INFO, "%s: Have device, timer was %p\n", pInfo->name, pIrtouch->reopen_timer);
		pIrtouch->reopen_timer = TimerSet(pIrtouch->reopen_timer, 0, 0, NULL, NULL);
		xf86FlushInput(pInfo->fd);
		xf86AddEnabledDevice(pInfo);
		device->public.on = TRUE;
	}
}

static void
IrtouchDeviceOff(DeviceIntPtr device)
{
	InputInfoPtr pInfo = device->public.devicePrivate;
	IrtouchDevicePtr pIrtouch = pInfo->private;

	xf86Msg(X_INFO, "%s: Off.\n", pInfo->name);
	if (pIrtouch->reopen_timer != NULL) {
		TimerFree(pIrtouch->reopen_timer);
		pIrtouch->reopen_timer = NULL;
	}
	xf86RemoveEnabledDevice(pInfo);
	device->public.on = FALSE;
}

static int IrtouchControl(DeviceIntPtr device, int what)
{
	InputInfoPtr pInfo = device->public.devicePrivate;
	IrtouchDevicePtr pIrtouch = pInfo->private;

	switch(what) {
		case DEVICE_INIT:
			_irtouch_init_buttons(device);
			_irtouch_init_axes(device);
			break;
		case DEVICE_ON:
			xf86Msg(X_INFO, "%s: On.\n", pInfo->name);
			IrtouchDeviceOn(device);
			break;
		case DEVICE_OFF:
			IrtouchDeviceOff(device);
			break;
		case DEVICE_CLOSE:
			break;
	}
	return Success;
}

static void IrtouchReadInput(InputInfoPtr pInfo)
{
	IrtouchDevicePtr pIrtouch = pInfo->private;
	uint8_t data[9];

	while(xf86WaitForInput(pInfo->fd, 0) > 0) {
		int n = read(pInfo->fd, data, sizeof(data));
		if (n <= 0) {
			if (errno == ENODEV) {
				xf86Msg(X_INFO, "%s: Device gone!\n", pInfo->name);
				xf86RemoveEnabledDevice(pInfo);
				close(pInfo->fd);
				pInfo->fd = -1;
				if (pIrtouch->reopen_timer != NULL) {
					pIrtouch->reopen_left = REOPEN_ATTEMPTS;
					pIrtouch->reopen_timer = TimerSet(pIrtouch->reopen_timer, 0, 100, IrtouchReopenTimer, pInfo);
				}
			} else if (errno != EAGAIN) {
				xf86Msg(X_ERROR, "%s: Read error.\n", pInfo->name);
				IrtouchDeviceOff(pInfo->dev);
				break;
			}
		}

		if (n != 9) {
			xf86Msg(X_WARNING, "%s: Incomplete packet received (length is %u), discarded.\n", pInfo->name, n);
			continue;
		}

		if (data[1] != 0xd0) {
			xf86Msg(X_WARNING, "%s: ID mismatch (was 0x%02x), ignoring packet.\n", pInfo->name, data[0]);
			continue;
		}

		int numtouches = data[2] >> 4;
		int curcoord = data[2] & 0xf;
		int x = data[3] << 8 | data[4];
		int y = data[6] << 8 | data[7];
#if 1
		//int newx = xf86ScaleAxis(x, MAX_RANGE, 0, pIrtouch->max_x, pIrtouch->min_x);
		//int newy = xf86ScaleAxis(y, MAX_RANGE, 0, pIrtouch->max_y, pIrtouch->min_y);
		int newx = ((float)(x - pIrtouch->min_x) / (float)(pIrtouch->max_x - pIrtouch->min_x)) * (float)MAX_RANGE;
		int newy = ((float)(y - pIrtouch->min_y) / (float)(pIrtouch->max_y - pIrtouch->min_y)) * (float)MAX_RANGE;
/*
		FILE* f = fopen("/tmp/a", "a+t");
		fprintf(f, "num=%u, curc=%u, x=%d, y=%d --> newx=%d, newy=%d\n", numtouches, curcoord, x, y, newx, newy);
		fclose(f);
*/
#endif

		xf86PostMotionEvent(pInfo->dev, TRUE, 0, 2, newx, newy);
		xf86PostButtonEvent(pInfo->dev, TRUE, 1, numtouches, 0, 0);
	}
}
