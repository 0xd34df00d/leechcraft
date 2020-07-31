/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QMenu>

class QAction;

namespace LC
{
namespace Poshuku
{
	class CustomWebView;
	class ProgressLineEdit;

	class IWebView;

	class UrlEditButtonsManager : public QObject
	{
		Q_OBJECT

		IWebView * const View_;
		ProgressLineEdit * const LineEdit_;
		QAction * const Add2Favorites_;

		const std::shared_ptr<QMenu> ExternalLinks_;
		QAction * const ExternalLinksAction_;
	public:
		UrlEditButtonsManager (IWebView*, ProgressLineEdit*, QAction*);
	private slots:
		void checkPageAsFavorite (const QString&);

		void checkLinkRels ();
		void showSendersMenu ();

		void updateBookmarksState ();
	};
}
}
