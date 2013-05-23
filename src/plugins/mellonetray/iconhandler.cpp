/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "iconhandler.h"
#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QtDebug>
#include <QTimer>
#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <util/x11/xwrapper.h>
#include "traymodel.h"

namespace LeechCraft
{
namespace Mellonetray
{
	const int Dim = 32;

	IconHandler::IconHandler (QGraphicsItem *item)
	: QGraphicsProxyWidget (item)
	, Proxy_ (new QLabel)
	, WID_ (0)
	, ThisWID_ (0)
	{
		Proxy_->show ();
		setWidget (Proxy_);
		connect (&TrayModel::Instance (),
				SIGNAL (updateRequired (ulong)),
				this,
				SLOT (checkUpdate (ulong)));
	}

	IconHandler::~IconHandler ()
	{
		Free ();
		delete Proxy_;
	}

	ulong IconHandler::GetWID () const
	{
		return WID_;
	}

	void IconHandler::SetWID (const ulong& wid)
	{
		if (wid == WID_)
			return;

		Free ();

		auto& w = Util::XWrapper::Instance ();

		auto disp = w.GetDisplay ();
		XWindowAttributes attrs;
		if (!XGetWindowAttributes (disp, wid, &attrs))
			return;

		auto visual = attrs.visual;
		XSetWindowAttributes setAttrs;
		setAttrs.colormap = attrs.colormap;
		setAttrs.background_pixel = 0;
		setAttrs.border_pixel = 0;
		ThisWID_ = XCreateWindow (disp, Proxy_->winId (),
				0, 0, Dim, Dim,
				0, attrs.depth, InputOutput, visual,
				CWColormap | CWBackPixel | CWBorderPixel,
				&setAttrs);

		XReparentWindow (disp, wid, ThisWID_, 0, 0);
		XSync (disp, false);

		XEvent ev;
		ev.xclient.type = ClientMessage;
		ev.xclient.serial = 0;
		ev.xclient.send_event = True;
		ev.xclient.message_type = w.GetAtom ("_XEMBED");
		ev.xclient.window = wid;
		ev.xclient.format = 32;
		ev.xclient.data.l [0] = CurrentTime;
		ev.xclient.data.l [1] = 0;
		ev.xclient.data.l [2] = 0;
		ev.xclient.data.l [3] = ThisWID_;
		ev.xclient.data.l [4] = 0;
		XSendEvent (disp, wid, false, 0xffffff, &ev);

		XSelectInput (disp, wid, StructureNotifyMask);

		XDamageCreate (disp, wid, XDamageReportRawRectangles);
		XCompositeRedirectWindow (disp, ThisWID_, CompositeRedirectManual);

		XMapWindow (disp, wid);
		XMapRaised (disp, ThisWID_);

		WID_ = wid;
		emit widChanged ();

		setGeometry (rect ());
		checkUpdate (wid);
	}

	void IconHandler::setGeometry (const QRectF& rect)
	{
		QGraphicsWidget::setGeometry (rect);

		if (ThisWID_ && WID_ && rect.width () * rect.height () > 0)
		{
			auto& w = Util::XWrapper::Instance ();
			w.ResizeWindow (ThisWID_, rect.width (), rect.height ());
			w.ResizeWindow (WID_, rect.width (), rect.height ());
			Proxy_->resize (rect.width (), rect.height ());
		}
	}

	void IconHandler::paint (QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w)
	{
		auto disp = Util::XWrapper::Instance ().GetDisplay ();

		XWindowAttributes attrs;
		if (!XGetWindowAttributes (disp, WID_, &attrs))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot get attributes";
			return;
		}

		auto ximage = XGetImage (disp, WID_, 0, 0, attrs.width, attrs.height, AllPlanes, ZPixmap);
		if (!ximage)
		{
			qWarning () << Q_FUNC_INFO
					<< "null image";
			return;
		}

		const QImage img (reinterpret_cast<const uchar*> (ximage->data),
				ximage->width, ximage->height, ximage->bytes_per_line, QImage::Format_ARGB32_Premultiplied);
		p->drawImage (opt->rect,
				img.scaled (opt->rect.size (), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}

	void IconHandler::Free ()
	{
		auto& w = Util::XWrapper::Instance ();
		auto disp = w.GetDisplay ();

		const bool shouldSync = WID_ || ThisWID_;

		if (WID_)
		{
			XSelectInput (disp, WID_, NoEventMask);
			XUnmapWindow (disp, WID_);
			XReparentWindow (disp, WID_, w.GetRootWindow (), 0, 0);
			WID_ = false;
		}

		if (ThisWID_)
		{
			XDestroyWindow (Util::XWrapper::Instance ().GetDisplay (), ThisWID_);
			ThisWID_ = 0;
		}

		if (shouldSync)
			XSync (disp, False);
	}

	void IconHandler::checkUpdate (ulong wid)
	{
		if (WID_ != wid)
			return;

		updateIcon ();
	}

	void IconHandler::updateIcon ()
	{
		update ();
	}
}
}
