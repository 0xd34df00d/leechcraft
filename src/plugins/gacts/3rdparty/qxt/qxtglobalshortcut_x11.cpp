/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#include "qxtglobalshortcut_p.h"
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <util/x11/xwrapper.h>
#include "keymapper_x11.h"

static int (*original_x_errhandler)(Display* display, XErrorEvent* event);

static int qxt_x_errhandler(Display* display, XErrorEvent *event)
{
    Q_UNUSED(display);
    switch (event->error_code)
    {
        case BadAccess:
        case BadValue:
        case BadWindow:
            if (event->request_code == 33 /* X_GrabKey */ ||
                event->request_code == 34 /* X_UngrabKey */)
            {
                QxtGlobalShortcutPrivate::error = true;
                //char errstr[256];
                //XGetErrorText(dpy, err->error_code, errstr, 256);
            }
			[[fallthrough]];
        default:
            return 0;
    }
}

bool QxtGlobalShortcutPrivate::nativeEventFilter (const QByteArray& eventType, void *msg, qintptr*)
{
	if (eventType != "xcb_generic_event_t")
		return false;

	const auto ev = static_cast<xcb_generic_event_t*> (msg);
	if ((ev->response_type & 127) != XCB_KEY_PRESS)
		return false;

	const auto kev = static_cast<xcb_key_press_event_t*> (msg);

	const auto keycode = kev->detail;
	uint keystate = 0;
	if (kev->state & XCB_MOD_MASK_1)
		keystate |= Mod1Mask;
	if (kev->state & XCB_MOD_MASK_CONTROL)
		keystate |= ControlMask;
	if (kev->state & XCB_MOD_MASK_4)
		keystate |= Mod4Mask;
	if (kev->state & XCB_MOD_MASK_SHIFT)
		keystate |= ShiftMask;

	activateShortcut (keycode, keystate & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask));

	return false;
}

quint32 QxtGlobalShortcutPrivate::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    // ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, and Mod5Mask
    quint32 native = 0;
    if (modifiers & Qt::ShiftModifier)
        native |= ShiftMask;
    if (modifiers & Qt::ControlModifier)
        native |= ControlMask;
    if (modifiers & Qt::AltModifier)
        native |= Mod1Mask;
    if (modifiers & Qt::MetaModifier)
        native |= Mod4Mask;
    return native;
}

std::pair<Display*, Window> GetX11DW ()
{
    const auto& inst = LC::Util::XWrapper::Instance ();
    return { inst.GetDisplay (), inst.GetRootWindow () };
}

quint32 QxtGlobalShortcutPrivate::nativeKeycode(Qt::Key key)
{
    // (davidsansome) Try the table from QKeyMapper first - this seems to be
    // the only way to get Keysyms for the media keys.
    unsigned int keysym = 0;
    int i = 0;
    while (KeyTbl[i]) {
      if (KeyTbl[i+1] == static_cast<uint>(key)) {
        keysym = KeyTbl[i];
        break;
      }
      i += 2;
    }

    // If that didn't work then fall back on XStringToKeysym
    if (!keysym) {
      keysym = XStringToKeysym(QKeySequence(key).toString().toLatin1().data());
    }

    return XKeysymToKeycode(GetX11DW ().first, keysym);
}

bool QxtGlobalShortcutPrivate::registerShortcut(quint32 nativeKey, quint32 nativeMods)
{
    auto [display, window] = GetX11DW ();
    Bool owner = True;
    int pointer = GrabModeAsync;
    int keyboard = GrabModeAsync;
    error = false;
    original_x_errhandler = XSetErrorHandler(qxt_x_errhandler);
    XGrabKey(display, nativeKey, nativeMods, window, owner, pointer, keyboard);
    XGrabKey(display, nativeKey, nativeMods | Mod2Mask, window, owner, pointer, keyboard); // allow numlock
    XSync(display, False);
    XSetErrorHandler(original_x_errhandler);
    return !error;
}

bool QxtGlobalShortcutPrivate::unregisterShortcut(quint32 nativeKey, quint32 nativeMods)
{
    auto [display, window] = GetX11DW ();
    error = false;
    original_x_errhandler = XSetErrorHandler(qxt_x_errhandler);
    XUngrabKey(display, nativeKey, nativeMods, window);
    XUngrabKey(display, nativeKey, nativeMods | Mod2Mask, window); // allow numlock
    XSync(display, False);
    XSetErrorHandler(original_x_errhandler);
    return !error;
}
