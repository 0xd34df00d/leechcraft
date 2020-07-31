/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/gui/findnotification.h>

class QWebEngineView;

namespace LC::Poshuku::WebEngineView
{
	class FindNotification : public Util::FindNotification
	{
		QWebEngineView * const WebView_;
		QString PreviousFindText_;
	public:
		FindNotification (ICoreProxy_ptr, QWebEngineView*);
	private:
		void ClearFindResults ();
	protected:
		void handleNext (const QString& text, FindFlags flags) override;
		void reject () override;
	};
}
