/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebEngineView>
#include "components/util/dnd.h"

namespace LC::Azoth
{
	class EntriesManager;

	class ChatTabWebView : public QWebEngineView
	{
		Q_OBJECT

		QAction *QuoteAct_ = nullptr;
		const EntriesManager *Entries_ = nullptr;
		DndUtil::DropHandler DropHandler_;
	public:
		using QWebEngineView::QWebEngineView;

		void InitializePage (QWebEngineProfile*, const EntriesManager&);
		void SetQuoteAction (QAction*);
		void SetDropHandler (DndUtil::DropHandler);
	protected:
		void contextMenuEvent (QContextMenuEvent*) override;

		void dragEnterEvent (QDragEnterEvent*) override;
		void dragMoveEvent (QDragMoveEvent*) override;
		void dragLeaveEvent (QDragLeaveEvent*) override;
		void dropEvent (QDropEvent*) override;
	private:
		void HandleNick (QMenu*, const QUrl&);
		void HandleURL (QMenu*, const QUrl&);
	signals:
		void linkClicked (const QUrl&, bool);
		void chatWindowSearchRequested (const QString&);
	};
}
