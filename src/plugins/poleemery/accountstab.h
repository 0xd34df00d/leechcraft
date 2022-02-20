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
#include "ui_accountstab.h"

class QStandardItemModel;

namespace LC
{
namespace Poleemery
{
	class AccountsManager;
	struct Account;

	class AccountsTab : public QWidget
					  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		AccountsManager *AccsManager_;

		const TabClassInfo TC_;
		QObject * const ParentPlugin_;

		Ui::AccountsTab Ui_;

		QStandardItemModel *AccsModel_;

		enum Roles
		{
			Acc = Qt::UserRole + 1
		};
	public:
		AccountsTab (const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void AddAccount (const Account&);
	private slots:
		void on_Add__released ();
		void on_Modify__released ();
		void on_Remove__released ();
	signals:
		void removeTab ();
	};
}
}
