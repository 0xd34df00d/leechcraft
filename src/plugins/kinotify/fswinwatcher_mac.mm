/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fswinwatcher.h"
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>

namespace LC::Kinotify
{
	bool IsCurrentWindowFullScreen ()
	{
		NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];
		if (!keyWindow)
			return false;

		return ([keyWindow styleMask] & NSWindowStyleMaskFullScreen) != 0;
	}
}
