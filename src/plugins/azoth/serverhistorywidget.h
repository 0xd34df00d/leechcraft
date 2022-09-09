/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "interfaces/azoth/ihaveserverhistory.h"
#include "tabbase.h"
#include "ui_serverhistorywidget.h"

class QSortFilterProxyModel;

namespace LC
{
namespace Azoth
{
	class IHaveServerHistory;

	class ServerHistoryWidget : public TabBase
	{
		Q_OBJECT

		Ui::ServerHistoryWidget Ui_;

		QToolBar * const Toolbar_;

		QObject * const AccObj_;
		IHaveServerHistory * const IHSH_;

		QByteArray CurrentID_;
		QByteArray MaxID_;
		int FirstMsgCount_ = -1;

		QSortFilterProxyModel * const ContactsFilter_;
	public:
		ServerHistoryWidget (QObject* account, QWidget* parent = nullptr);

		void Remove () override;
		QToolBar* GetToolBar () const override;

		void SelectEntry (ICLEntry*);
	private:
		int GetReqMsgCount () const;
	private slots:
		void handleFetched (const QModelIndex&, const QByteArray&, const SrvHistMessages_t&);
		void on_ContactsView__clicked (const QModelIndex&);
		void on_MessagesView__anchorClicked (const QUrl&);
		void navigatePrevious ();
		void navigateNext ();
	};
}
}
