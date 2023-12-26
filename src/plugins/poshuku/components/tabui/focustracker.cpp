/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "focustracker.h"
#include <QKeyEvent>
#include <QLineEdit>
#include <QTimer>
#include <QtDebug>
#include <util/sll/lambdaeventfilter.h>

namespace LC::Poshuku
{
	FocusTracker::FocusTracker (QLineEdit& urlEdit, const IWebView_ptr& view, QWidget& tab)
	: QObject { &tab }
	, UrlEdit_ { urlEdit }
	, ViewWidget_ { *view->GetQWidget () }
	{
		connect (&urlEdit,
				&QLineEdit::textEdited,
				this,
				[this] { FocusOnLoad_ = false; });
		connect (&urlEdit,
				&QLineEdit::editingFinished,
				this,
				[this] { FocusOnLoad_ = true; });

		connect (&ViewWidget_,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (handleLoadFinished ()));

		tab.installEventFilter (Util::MakeLambdaEventFilter<QEvent::KeyRelease> ([this] (QKeyEvent *e)
				{
					if (e->key () == Qt::Key_F6)
					{
						FocusUrlLine ();
						FocusOnLoad_ = false;
						return true;
					}
					return false;
				}, *this));
		QTimer::singleShot (0, this, &FocusTracker::FocusUrlLine);
	}

	void FocusTracker::FocusUrlLine ()
	{
		UrlEdit_.setFocus (Qt::OtherFocusReason);
		UrlEdit_.selectAll ();
	}

	void FocusTracker::handleLoadFinished ()
	{
		if (FocusOnLoad_ && ViewWidget_.isVisible ())
			ViewWidget_.setFocus ();
	}
}
