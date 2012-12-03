/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "fullscreen.h"
#include <AppKit/NSView.h>
#include <AppKit/NSWindow.h>
#include <Foundation/NSNotification.h>
#include <QMainWindow>
#include <QSysInfo>
#include <QtGlobal>

@interface WindowObserver : NSObject
{
	QMainWindow *Window_;
}

- (id) initWithMainWindow:(QMainWindow*)w;

- (void) notifyDidEnterFullScreen:(NSNotification*)notification;
- (void) notifyDidExitFullScreen:(NSNotification*)notification;

@end

@implementation WindowObserver

- (id) initWithMainWindow:(QMainWindow*)w;
{
	if ((self = [self init]))
		Window_ = w;
	return self;
}

- (void) notifyDidEnterFullScreen:(NSNotification*)notification
{
	Q_UNUSED(notification)
}

- (void) notifyDidExitFullScreen:(NSNotification*)notification
{
	Q_UNUSED(notification)
}

@end

static WindowObserver *observer = nil;

static bool SupportsFSImpl ()
{
#if QT_VERSION >= 0x040800
	return QSysInfo::MacintoshVersion >= QSysInfo::MV_LION;
#else
	return QSysInfo::MacintoshVersion >= 0x0009; /* MV_LION not defined */
#endif
}

static void AddActionImpl (QMainWindow *window)
{
	if (!SupportsFSImpl ())
		return;

	NSView *nsview = (NSView *) window->winId();
	NSWindow *nswindow = [nsview window];
	[nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

	if (observer == nil)
		observer = [[WindowObserver alloc] initWithMainWindow:window];
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:observer selector:@selector(notifyDidEnterFullScreen:)
		name:NSWindowDidEnterFullScreenNotification object:nswindow];
	[nc addObserver:observer selector:@selector(notifyDidExitFullScreen:)
		name:NSWindowDidExitFullScreenNotification object:nswindow];
}

static void ToggleImpl (QMainWindow *window)
{
	if (!SupportsFSImpl ())
		return;

	NSView *nsview = (NSView *) window->winId();
	NSWindow *nswindow = [nsview window];
	[nswindow performSelector:@selector(toggleFullScreen:) withObject: nil];
}

namespace LeechCraft
{
namespace Pierre
{
namespace FS
{
	bool SupportsFS ()
	{
		return SupportsFSImpl ();
	}

	void AddAction (QMainWindow *w)
	{
		AddActionImpl (w);
	}

	void Toggle (QMainWindow *w)
	{
		ToggleImpl (w);
	}
}
}
}
