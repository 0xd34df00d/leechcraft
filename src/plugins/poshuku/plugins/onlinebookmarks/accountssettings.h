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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_SETTINGS_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_SETTINGS_H

#include <QWidget>
#include <interfaces/ibookmarksservice.h>
#include <interfaces/iauthwidget.h>
#include "ui_accountssettings.h"

class QStandardItemModel;

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

		QStandardItemModel *AccountsModel_;
		QHash<IBookmarksService*, QWidget*> Service2AuthWidget_;
	public:
		enum ServiceObject
		{
			RServiceObject = Qt::UserRole + 1
		};

		AccountsSettings ();
		~AccountsSettings ();
	public slots:
		void accept ();
	private slots:
		void on_Add__toggled (bool);
		void on_Edit__toggled (bool);
		void on_Delete__clicked ();
		void on_AccountsView__clicked (const QModelIndex&);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_SETTINGS_H
