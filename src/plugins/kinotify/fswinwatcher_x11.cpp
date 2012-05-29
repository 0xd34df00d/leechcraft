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

#include "fswinwatcher.h"
#include <X11/Xlib.h>
#include <X11/extensions/randr.h>
#include <QX11Info>
#include <QMainWindow>


namespace LeechCraft
{
namespace Kinotify
{
	FSWinWatcher::FSWinWatcher (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
	}


	bool getSize(Display *dpy, Window win, int *width,int *height){
		XWindowAttributes windowattr; 
		if (XGetWindowAttributes(dpy, win, &windowattr) == 0) 
			return false;
		*width = windowattr.width;
		*height = windowattr.height;
		return true;
	}

	bool FSWinWatcher::IsCurrentFS ()
	{
		Display * display;
		Window focusWin;
		int reverToReturn;
		int num_sizes,screen;
		int screenWidth,screenHeight,width,height;
		Rotation original_rotation;
		display = QX11Info::display ();
		if (!display) return false;
		screen = QX11Info::appScreen ();
		XGetInputFocus(display, &focusWin, &reverToReturn);
		if ((Proxy_->GetMainWindow ()->effectiveWinId ()) == focusWin)
			return false;
		if (! (getSize(display, RootWindow(display, screen), &screenWidth, &screenHeight)
			&& getSize(display, focusWin, &width, &height)))
			return false;

		return (screenWidth==width) && (screenHeight==height);
	}

}
}
