/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QMap>
#include "ui_bookmarksmanagerdialog.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
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
		QStandardItemModel *BMModel_;

		IAccount *CurrentAccount_ = nullptr;
	public:
		BookmarksManagerDialog (QWidget* = 0);

		bool FocusOn (IAccount*);
		void SuggestSaving (QObject*);
	private:
		void Save ();
		QStandardItem* GetSelectedItem () const;
		void ReloadModel ();

		void AddBookmark (const QVariantMap&);
	private slots:
		void on_AccountBox__currentIndexChanged (int);
		void handleBookmarksChanged ();
		void on_RemoveButton__released ();
		void on_AddButton__released ();
		void on_ModifyButton__released ();
		void on_MoveUp__released ();
		void on_MoveDown__released ();
	};
}
}
