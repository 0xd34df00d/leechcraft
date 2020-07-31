/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historyvieweventfilter.h"
#include <QTextBrowser>
#include <QMouseEvent>

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	HistoryViewEventFilter::HistoryViewEventFilter (QTextBrowser *browser)
	: QObject { browser }
	, Viewer_ { browser }
	{
		Viewer_->installEventFilter (this);
		for (const auto child : Viewer_->findChildren<QWidget*> ())
			child->installEventFilter (this);
	}

	bool HistoryViewEventFilter::eventFilter (QObject*, QEvent *event)
	{
		if (event->type () != QEvent::MouseButtonRelease)
			return false;

		const auto mev = static_cast<QMouseEvent*> (event);
		if (!(mev->button () & Qt::MiddleButton))
			return false;

		const auto& text = Viewer_->anchorAt (mev->pos ());
		emit bgLinkRequested (QUrl { text });
		return false;
	}
}
}
}

