/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "findnotification.h"
#include <QWebEnginePage>
#include <QWebEngineView>

namespace LC::Util
{
	/** @brief A helper class to aid connecting FindNotification with
	 * QtWebEngine.
	 *
	 * This class is basically a FindNotification providing an utility
	 * function ToPageFlags() to convert FindNotification::FindFlags to
	 * QWebEnginePage::FindFlags.
	 *
	 * FindNotificationWE takes care of all the search-related things
	 * and automatically handles the QWebView passed to its constructor
	 * in its handleNext() implementation. So, using this class is as
	 * simple as just instantiating an object, passing the needed
	 * QWebView instance to its constructor.
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API FindNotificationWE : public FindNotification
	{
		QWebEngineView * const WebView_;
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
		FindNotificationWE (const ICoreProxy_ptr& proxy, QWebEngineView *near)
		: FindNotification { proxy, near }
		, WebView_ { near }
		{
		}

		/** @brief Converts the given \em findFlags to WebKit find flags.
		 *
		 * @param[in] findFlags The find flags in terms of
		 * FindNotification.
		 * @return The find flags in terms of WebKit.
		 */
		static QWebEnginePage::FindFlags ToPageFlags (FindFlags findFlags)
		{
			QWebEnginePage::FindFlags pageFlags;
			auto check = [&pageFlags, findFlags] (FindFlag ourFlag, QWebEnginePage::FindFlag pageFlag)
			{
				if (findFlags & ourFlag)
					pageFlags |= pageFlag;
			};
			check (FindCaseSensitively, QWebEnginePage::FindCaseSensitively);
			check (FindBackwards, QWebEnginePage::FindBackward);
			return pageFlags;
		}
	private:
		void ClearFindResults ()
		{
			PreviousFindText_.clear ();
			WebView_->page ()->findText ({});
		}
	protected:
		void HandleNext (const QString& text, FindFlags findFlags) override
		{
			if (PreviousFindText_ != text)
			{
				ClearFindResults ();
				PreviousFindText_ = text;
			}

			WebView_->page ()->findText (text, ToPageFlags (findFlags),
					[this] (bool found) { SetSuccessful (found); });
		}

		void Reject () override
		{
			FindNotification::Reject ();
			ClearFindResults ();
		}
	};
}
