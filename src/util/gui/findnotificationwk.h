/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "findnotification.h"
#include <QWebPage>
#include <QWebView>

namespace LC::Util
{
	/** @brief A helper class to aid connecting FindNotification with
	 * Qt WebKit.
	 *
	 * This class is basically a FindNotification providing an utility
	 * function ToPageFlags() to convert FindNotification::FindFlags to
	 * QWebPage::FindFlags.
	 *
	 * FindNotificationWk takes care of all the search-related things
	 * and automatically handles the QWebView passed to its constructor
	 * in its handleNext() implementation. So, using this class is as
	 * simple as just instantiating an object, passing the needed
	 * QWebView instance to its constructor.
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API FindNotificationWk : public FindNotification
	{
		QWebView * const WebView_;
		QString PreviousFindText_;
	public:
		/** @brief Constructs the find notification using the given
		 * proxy and near widget.
		 *
		 * @param[in] proxy The core proxy to be used by this find
		 * notification.
		 * @param[in] near The web view near which to embed.
		 *
		 * @sa FindNotification
		 */
		FindNotificationWk (const ICoreProxy_ptr& proxy, QWebView *near)
		: FindNotification { proxy, near }
		, WebView_ { near }
		{
			connect (near,
					&QWebView::loadFinished,
					this,
					[this]
					{
						if (PreviousFindText_.isEmpty ())
							return;

						ClearFindResults ();
						FindNext ();
					});
		}

		/** @brief Converts the given \em findFlags to WebKit find flags.
		 *
		 * @param[in] findFlags The find flags in terms of
		 * FindNotification.
		 * @return The find flags in terms of WebKit.
		 */
		static QWebPage::FindFlags ToPageFlags (FindFlags findFlags)
		{
			QWebPage::FindFlags pageFlags;
			auto check = [&pageFlags, findFlags] (FindFlag ourFlag, QWebPage::FindFlag pageFlag)
			{
				if (findFlags & ourFlag)
					pageFlags |= pageFlag;
			};
			check (FindCaseSensitively, QWebPage::FindCaseSensitively);
			check (FindBackwards, QWebPage::FindBackward);
			check (FindWrapsAround, QWebPage::FindWrapsAroundDocument);
			return pageFlags;
		}
	private:
		void ClearFindResults ()
		{
			PreviousFindText_.clear ();
			WebView_->page ()->findText ({}, QWebPage::HighlightAllOccurrences);
		}
	protected:
		void HandleNext (const QString& text, FindFlags findFlags) override
		{
			const auto flags = ToPageFlags (findFlags);

			if (PreviousFindText_ != text)
			{
				const auto nflags = flags | QWebPage::HighlightAllOccurrences;
				WebView_->page ()->findText ({}, nflags);
				WebView_->page ()->findText (text, nflags);
				PreviousFindText_ = text;
			}

			const auto found = WebView_->page ()->findText (text, flags);
			SetSuccessful (found);
		}

		void Reject () override
		{
			FindNotification::Reject ();
			ClearFindResults ();
		}
	};
}
