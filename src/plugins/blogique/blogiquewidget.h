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

		enum Columns
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

		QHash<QStandardItem*, Entry> DraftItem2Entry_;
		QHash<QStandardItem*, Entry> PostItem2Entry_;

		QAction *OpenDraftInNewTab_;
		QAction *OpenDraftInCurrentTab_;
		QAction *OpenEntryInNewTab_;
		QAction *OpenEntryInCurrentTab_;

		QAction *LoadLocalEntries_;
		QList<QAction*> LoadActions_;

		qlonglong DraftID_;
		qlonglong EventID_;
	public:
		BlogiqueWidget (QWidget *parent = 0);

		QObject* ParentMultiTabs ();
		TabClassInfo GetTabClassInfo () const;
		QToolBar* GetToolBar () const;
		void Remove ();

		void FillWidget (const Entry& e, bool isDraft = false,
				const QByteArray& accId = QByteArray ());

		static void SetParentMultiTabs (QObject *tab);
	private:
		void SetTextEditor ();
		void SetToolBarActions ();
		void SetDeafultSideWidgets ();
		void RemovePostingTargetsWidget ();
		void FillPostingStatistic ();

		void ClearEntry ();

		QList<QStandardItem*> CreateItemsToView (const Entry& entry) const;

		Entry GetCurrentEntry ();

		void LoadDrafts ();
		Entry LoadFullDraft (qlonglong draftID);
		void RemoveDraft (qlonglong id);

		void LoadEntries ();
		Entry LoadEntry (qlonglong Id);

		void FillPostsView (const QList<Entry> entries);
		void FillDraftsView (const QList<Entry> entries);

	private slots:
		void handleCurrentAccountChanged (int id);
		void newEntry ();
		void saveEntry ();
		void saveNewEntry ();
		void submit (const Entry& e = Entry ());
		void saveSplitterPosition (int, int);
		void on_SideWidget__dockLocationChanged (Qt::DockWidgetArea area);
		void on_UpdateProfile__triggered ();
		void on_RemoveDraft__released ();
		void on_PublishDraft__released ();
		void on_RemoveRemotePost__released ();
		void on_Edit__released ();
		void on_PostsView__doubleClicked (const QModelIndex& index);
		void handleOpenEntryInCurrentTab (const QModelIndex& index = QModelIndex ());
		void handleOpenEntryInNewTab (const QModelIndex& index = QModelIndex ());
		void on_LocalEntriesView__doubleClicked (const QModelIndex& index);
		void handleOpenDraftInCurrentTab (const QModelIndex& index = QModelIndex ());
		void handleOpenDraftInNewTab (const QModelIndex& index = QModelIndex ());

		void loadPostsByDate (const QDate& date);

		void handleGotEntries (const QList<Entry>& entries = QList<Entry> ());
		void handleStorageUpdated ();

		void loadLocalEntries ();
		void handleLoadEntries (const QList<Entry>& entries);

	signals:
		void removeTab (QWidget *tab);
		void addNewTab (const QString& name, QWidget *tab);
	};
}
}
