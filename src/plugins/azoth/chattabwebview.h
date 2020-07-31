/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebView>

namespace LC
{
namespace Azoth
{
	class ChatTabWebView : public QWebView
	{
		Q_OBJECT

		QAction *QuoteAct_;
	public:
		ChatTabWebView (QWidget* = 0);

		void SetQuoteAction (QAction*);
	protected:
		void mouseReleaseEvent (QMouseEvent*);
		void contextMenuEvent (QContextMenuEvent*);
	private:
		void HandleNick (QMenu*, const QUrl&);
		void HandleURL (QMenu*, const QUrl&);
		void HandleDataFilters (QMenu*, const QString&);
	private slots:
		void handleOpenLink ();
		void handleOpenExternally ();
		void handleOpenAsURL ();
		void handleHighlightOccurences ();
		void handleSaveLink ();
		void handlePageLinkClicked (const QUrl&);
	signals:
		void linkClicked (const QUrl&, bool);
		void chatWindowSearchRequested (const QString&);
	};
}
}
