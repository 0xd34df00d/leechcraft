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

#ifndef PLUGINS_AZOTH_BOOKMARKSMANAGERDIALOG_H
#define PLUGINS_AZOTH_BOOKMARKSMANAGERDIALOG_H
#include <QDialog>
#include <QMap>
#include "ui_bookmarksmanagerdialog.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace Azoth
{
	class IMUCJoinWidget;
	class IMUCBookmarkEditorWidget;
	class IAccount;

	class BookmarksManagerDialog : public QDialog
	{
		Q_OBJECT

		Ui::BookmarksManagerDialog Ui_;
		QMap<QByteArray, IMUCJoinWidget*> Proto2Joiner_;
		QStandardItemModel *BMModel_;
		IMUCBookmarkEditorWidget *CurrentEditor_;
	public:
		BookmarksManagerDialog (QWidget* = 0);

		void FocusOn (IAccount*);
		void SuggestSaving (QObject*);
	private:
		void Save ();
		bool CheckSave (const QModelIndex&);
		QStandardItem* GetSelectedItem () const;
	private slots:
		void on_AccountBox__currentIndexChanged (int);
		void handleBookmarksChanged ();
		void handleCurrentBMChanged (const QModelIndex&, const QModelIndex&);
		void on_RemoveButton__released ();
		void on_AddButton__released ();
		void on_ApplyButton__released ();
		void on_MoveUp__released ();
		void on_MoveDown__released ();
	};
}
}

#endif
