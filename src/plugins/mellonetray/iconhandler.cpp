/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "iconhandler.h"
#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QQuickWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtDebug>
#include "traymodel.h"

namespace LC
{
namespace Mellonetray
{
	IconHandler::IconHandler (QQuickItem *item)
	: QQuickItem (item)
	{
		setFlag (QQuickItem::ItemHasContents);
	}

	ulong IconHandler::GetWID () const
	{
		return WID_;
	}

	void IconHandler::SetWID (const ulong& wid)
	{
		if (wid == WID_)
			return;

		Proxy_.reset ();

		WID_ = wid;
		emit widChanged ();
	}

	void IconHandler::geometryChanged (const QRectF& rect, const QRectF& oldRect)
	{
		QQuickItem::geometryChanged (rect, oldRect);

		if (!window ())
			return;

		if (!Proxy_ && WID_)
		{
			Proxy_.reset (QWindow::fromWinId (WID_));
			Proxy_->setPosition (-1024, -1024);
			Proxy_->show ();
		}

		if (Proxy_ && rect.width () * rect.height () > 0)
		{
			Proxy_->resize (rect.width (), rect.height ());

			const auto& scenePoint = mapToScene ({ 0, 0 }).toPoint ();
			Proxy_->setPosition (window ()->mapToGlobal (scenePoint));
		}
	}
}
}
