/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "interfaces/blogique/iaccount.h"
#include "ui_blogiquewidget.h"

class QStandardItem;
class QStandardItemModel;
class IEditorWidget;
class QToolBar;
class QComboBox;

namespace LeechCraft
{
namespace Blogique
{
	class IBlogiqueSideWidget;
	class IAccount;
	class LocalStorage;

	class BlogiqueWidget : public QWidget
						,  public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		enum BlogiqueSideWidgets
		{
			PostOptionsWidget = 2
		};

		enum EntryIdRole
		{
			DBIdRole = Qt::UserRole + 1
		};

		enum DraftColumns
		{
			Date,
			Subject
		};

		static QObject *S_ParentMultiTabs_;

		Ui::BlogiqueWidget Ui_;

		IEditorWidget *PostEdit_;
		QWidget *PostEditWidget_;
		QToolBar *ToolBar_;
		QComboBox *AccountsBox_;
		QComboBox *PostTargetBox_;
		QAction *PostTargetAction_;

		QHash<int, IAccount*> Id2Account_;
		int PrevAccountId_;
		QList<QWidget*> SidePluginsWidgets_;

		QStandardItemModel *PostsViewModel_;
		QStandardItemModel *DraftsViewModel_;

		LocalStorage *Storage_;

		QHash<QStandardItem*, Event> DraftItem2Event_;

		QAction *OpenDraftInNewTab_;
		QAction *OpenDraftInCurrentTab_;

		qlonglong DraftID_;
	public:
		BlogiqueWidget (QWidget *parent = 0);

		QObject* ParentMultiTabs ();
		TabClassInfo GetTabClassInfo () const;
		QToolBar* GetToolBar () const;
		void Remove ();

		void FillWidget (const Event& e, const QByteArray& accId = QByteArray ());

		static void SetParentMultiTabs (QObject *tab);
	private:
		void RemovePostingTargetsWidget ();
		Event GetCurrentEvent ();
		void LoadDrafts ();
		Event LoadFullDraft (const QByteArray& id, qlonglong draftID);
		void RemoveDraft (qlonglong id);

	private slots:
		void handleCurrentAccountChanged (int id);
		void saveEntry ();
		void saveNewEntry ();
		void submit (const Event& e = Event ());
		void saveSplitterPosition (int, int);
		void on_UpdateProfile__triggered ();
		void on_RemoveDraft__released ();
		void on_PublishDraft__released ();
		void on_LocalEntriesView__doubleClicked (const QModelIndex& index);
		void handleOpenInCurrentTab (const QModelIndex& index = QModelIndex ());
		void handleOpenInNewTab (const QModelIndex& index = QModelIndex ());

	signals:
		void removeTab (QWidget *tab);
		void addNewTab (const QString& name, QWidget *tab);
	};
}
}
