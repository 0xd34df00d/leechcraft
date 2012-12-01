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

#include "dockutil.h"
#include <QString>
#include <AppKit/NSApplication.h>
#include <AppKit/NSDockTile.h>
#include <Foundation/NSString.h>

static NSString* toNsString (const QString& text)
{
	auto utf8String = text.toUtf8 ().constData ();
	return [[NSString alloc] initWithUTF8String: utf8String];
}

static void SetDockBadgeImpl (const QString& text)
{
	auto badgeString = toNsString (text);
	[[NSApp dockTile] setBadgeLabel: badgeString];
	[badgeString release];
}

namespace LeechCraft
{
namespace Pierre
{
namespace DU
{
	void SetDockBadge (const QString& text)
	{
		SetDockBadgeImpl (text);
	}
}
}
}
