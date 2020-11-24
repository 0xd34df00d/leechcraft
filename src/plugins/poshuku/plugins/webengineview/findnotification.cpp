/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "findnotification.h"
#include <QWebEngineView>

namespace LC::Poshuku::WebEngineView
{
	FindNotification::FindNotification (ICoreProxy_ptr proxy, QWebEngineView *near)
	: Util::FindNotification { proxy, near }
	, WebView_ { near }
	{
		connect (near,
				&QWebEngineView::loadFinished,
				this,
				[this]
				{
					if (PreviousFindText_.isEmpty ())
						return;

					ClearFindResults ();
					FindNext ();
				});
	}

	void FindNotification::ClearFindResults ()
	{
		PreviousFindText_ = "";
		WebView_->findText ({});
	}

	namespace
	{
		auto ToPageFlags (FindNotification::FindFlags flags)
		{
			QWebEnginePage::FindFlags pageFlags;
			auto check = [&pageFlags, flags] (FindNotification::FindFlag ourFlag, QWebEnginePage::FindFlag pageFlag)
			{
				if (flags & ourFlag)
					pageFlags |= pageFlag;
			};
			check (FindNotification::FindCaseSensitively, QWebEnginePage::FindCaseSensitively);
			check (FindNotification::FindBackwards, QWebEnginePage::FindBackward);
			return pageFlags;
		}
	}

	void FindNotification::HandleNext (const QString& text, FindFlags findFlags)
	{
		const auto flags = ToPageFlags (findFlags);

		if (PreviousFindText_ != text)
		{
			WebView_->findText ({}, flags);
			PreviousFindText_ = text;
		}

		WebView_->findText (text, flags, [this] (bool found) { SetSuccessful (found); });
	}

	void FindNotification::Reject ()
	{
		Util::FindNotification::Reject ();
		ClearFindResults ();
	}
}
