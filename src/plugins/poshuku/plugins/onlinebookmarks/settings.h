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
#include "interfaces/structures.h"
#include "ui_accounts.h"

class QStandardItemModel;
class QFrame;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QModelIndex;

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class Core;
	class AbstractBookmarksService;

	class Settings : public QWidget
	{
		Q_OBJECT

		Ui::Accounts_ Ui_;
		QStandardItemModel *Model_;
		QList<AbstractBookmarksService*> BookmarksServices_;
	public:
		Settings (QStandardItemModel*);
		QString GetSelectedName () const;
		void SetConfirmSend (bool);
	private:
		void ClearFrameState ();
		void SetupServices ();
		void ReadSettings ();
		void SetApplyEnabled (const QString&, const QString&);
	public slots:
		void accept ();
	private slots:
		void on_Add__toggled (bool);
		void on_Edit__toggled (bool);
		void on_Delete__released ();
		void handleStuff ();
		void handleLoginTextChanged (const QString&);
		void handlePasswordTextChanged (const QString&);
		void on_AccountsView__clicked (const QModelIndex&);
		void checkServiceAnswer (bool);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_SETTINGS_H
