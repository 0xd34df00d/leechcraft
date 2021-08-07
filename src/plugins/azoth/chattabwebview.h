/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebEngineView>

namespace LC::Azoth
{
	class ChatTabWebView : public QWebEngineView
	{
		Q_OBJECT

		QAction *QuoteAct_ = nullptr;
	public:
		explicit ChatTabWebView (QWidget* = nullptr);

		void SetQuoteAction (QAction*);
	protected:
		void contextMenuEvent (QContextMenuEvent*) override;
	private:
		void HandleNick (QMenu*, const QUrl&);
		void HandleURL (QMenu*, const QUrl&);
	signals:
		void linkClicked (const QUrl&, bool);
		void chatWindowSearchRequested (const QString&);
	};
}
