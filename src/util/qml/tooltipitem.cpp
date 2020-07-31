/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tooltipitem.h"
#include <QToolTip>
#include <QtDebug>

namespace LC
{
namespace Util
{
	ToolTipItem::ToolTipItem (QQuickItem *parent)
	: QQuickItem (parent)
	{
		setAcceptHoverEvents (true);
		connect (&ShowTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (showToolTip ()));
		ShowTimer_.setSingleShot (true);
	}

	void ToolTipItem::SetText (const QString& text)
	{
		if (Text_ != text)
		{
			Text_ = text;
			emit textChanged ();
		}
	}

	QString ToolTipItem::GetText () const
	{
		return Text_;
	}

	bool ToolTipItem::ContainsMouse () const
	{
		return ContainsMouse_;
	}

	void ToolTipItem::ShowToolTip (const QString& text) const
	{
		QToolTip::showText (cursor ().pos (), text);
	}

	void ToolTipItem::hoverEnterEvent (QHoverEvent *event)
	{
		ShowTimer_.start (1000);
		ContainsMouse_ = true;
		emit containsMouseChanged ();
		QQuickItem::hoverEnterEvent (event);
	}

	void ToolTipItem::hoverLeaveEvent (QHoverEvent *event)
	{
		ShowTimer_.stop ();
		ContainsMouse_ = false;
		emit containsMouseChanged ();
		QQuickItem::hoverLeaveEvent (event);
	}

	void ToolTipItem::showToolTip ()
	{
		ShowToolTip (Text_);
	}
}
}
