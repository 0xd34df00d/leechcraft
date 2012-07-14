/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ACCOUNTSETTINGS_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ACCOUNTSETTINGS_H

#include <QWidget>
#include <util/util.h>
#include <interfaces/ibookmarksservice.h>
#include "ui_accountssettings.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class AccountsSettings : public QWidget
	{
		Q_OBJECT

		Ui::AccountsSettings Ui_;

		QHash<QStandardItem*, IBookmarksService*> Item2Service_;
		QHash<QStandardItem*, IAccount*> Item2Account_;
		QHash<QByteArray, QObject*> Id2Account_;
		QHash<IBookmarksService*, QWidget*> Service2AuthWidget_;
		QStandardItemModel *AccountsModel_;
		bool Scheduled_;
		QWidget *LastWidget_;
		QHash<QAction*, IBookmarksService*> Action2Service_;
	public:
		AccountsSettings ();
		~AccountsSettings ();

		void InitServices ();
		QStandardItemModel* GetAccountsModel () const;
		void UpdateDates ();
	private:
		QModelIndex GetServiceIndex (QObject*) const;
		void ScheduleResize ();
	public slots:
		void accept ();
	private slots:
		void resizeColumns ();

		void on_Delete__clicked ();
		void on_Auth__clicked ();
		void on_Register__clicked ();
		void on_AccountsView__clicked (const QModelIndex&);
		void on_AddAccount__triggered (QAction*);
		void on_Close__clicked ();

		void addAccount (QObjectList);
	signals:
		void accountRemoved (QObject*);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ACCOUNTSETTINGS_H
