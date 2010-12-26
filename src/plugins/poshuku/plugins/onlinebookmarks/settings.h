/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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
#include "ui_settings.h"

class QStandardItemModel;
class QFrame;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QModelIndex;

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace OnlineBookmarks
{
	class OnlineBookmarks;
	class AbstractBookmarksService;
	
	class Settings : public QWidget
	{
		Q_OBJECT

		Ui::Settings_ Ui_;
		OnlineBookmarks *OnlineBookmarks_;
		QStandardItemModel *Model_;
		QStandardItemModel *ServicesModel_;
		QFrame *LoginFrame_;
		QCheckBox *YahooID_;
		QPushButton *Apply_;
		QLineEdit *Login_;
		QLineEdit *Password_;
		QList<AbstractBookmarksService*> BookmarksServices_;
	public:
		Settings (QStandardItemModel*, OnlineBookmarks*);
		QString GetSelectedName () const;
	private:
		QFrame* CreateLoginWidget (QWidget *parent = 0);
		void ClearFrameState ();
		void SetupServices ();
		void SetPassword (const QString&, const QString&, const QString&);
		QString GetPassword (const QString&, const QString&);
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
		void on_Services__currentIndexChanged (const QString&);
		void on_AccauntsView__clicked (const QModelIndex&);
		void checkServiceAnswer (bool);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
};
};
};
};
};
#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_SETTINGS_H
