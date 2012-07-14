/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_PRIVACYLISTSCONFIGDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_PRIVACYLISTSCONFIGDIALOG_H
#include <QDialog>
#include "ui_privacylistsconfigdialog.h"
#include "privacylistsmanager.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class PrivacyListsManager;

	class PrivacyListsConfigDialog : public QDialog
	{
		Q_OBJECT

		Ui::PrivacyListsConfigDialog Ui_;
		PrivacyListsManager *Manager_;
		QMap<QString, PrivacyList> Lists_;
		
		QStandardItemModel *Model_;
	public:
		PrivacyListsConfigDialog (PrivacyListsManager*, QWidget* = 0);
	private:
		void QueryLists ();
		void QueryList (const QString&);
		void AddListToBoxes (const QString&);
		void ReinitModel ();
		QList<QStandardItem*> ToRow (const PrivacyListItem&) const;
	public slots:
		void accept ();
		void reject ();
	private slots:
		void on_ConfigureList__activated (int);
		void on_AddButton__released ();
		void on_RemoveButton__released ();
		void on_DefaultPolicy__currentIndexChanged (int);
		void on_AddRule__released ();
		void on_ModifyRule__released ();
		void on_RemoveRule__released ();
		void on_MoveUp__released ();
		void on_MoveDown__released ();
		void handleGotLists (const QStringList&, const QString&, const QString&);
		void handleGotList (const PrivacyList&);
	};
}
}
}

#endif
