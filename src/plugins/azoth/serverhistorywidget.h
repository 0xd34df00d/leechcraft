/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "interfaces/azoth/ihaveserverhistory.h"
#include "ui_serverhistorywidget.h"

class QSortFilterProxyModel;

namespace LC
{
namespace Azoth
{
	class IHaveServerHistory;

	class ServerHistoryWidget : public QWidget
							  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		QObject *PluginObj_;
		TabClassInfo TC_;

		Ui::ServerHistoryWidget Ui_;

		QToolBar * const Toolbar_;

		QObject * const AccObj_;
		IHaveServerHistory * const IHSH_;

		QByteArray CurrentID_;
		QByteArray MaxID_;
		int FirstMsgCount_ = -1;

		QSortFilterProxyModel * const ContactsFilter_;
	public:
		ServerHistoryWidget (QObject*, QWidget* = nullptr);

		void SetTabInfo (QObject*, const TabClassInfo&);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		void SelectEntry (ICLEntry*);
	private:
		int GetReqMsgCount () const;
	private slots:
		void handleFetched (const QModelIndex&, const QByteArray&, const SrvHistMessages_t&);
		void on_ContactsView__clicked (const QModelIndex&);
		void on_MessagesView__anchorClicked (const QUrl&);
		void navigatePrevious ();
		void navigateNext ();
	signals:
		void removeTab (QWidget*);
	};
}
}
