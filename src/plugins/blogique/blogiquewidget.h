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
	class LocalEntriesWidget;
	class RemoteEntriesWidget;

	class BlogiqueWidget : public QWidget
						,  public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		enum BlogiqueSideWidgets
		{
			PostOptionsWidget = 2
		};

		static QObject *S_ParentMultiTabs_;

		Ui::BlogiqueWidget Ui_;

		IEditorWidget *PostEdit_;
		QWidget *PostEditWidget_;
		QToolBar *ToolBar_;
		QComboBox *AccountsBox_;
		QComboBox *PostTargetBox_;
		QAction *PostTargetAction_;

		LocalEntriesWidget *LocalEntriesWidget_;
		RemoteEntriesWidget *RemoteEntriesWidget_;
		QHash<int, IAccount*> Id2Account_;
		int PrevAccountId_;
		QList<QWidget*> SidePluginsWidgets_;

	public:
		BlogiqueWidget (QWidget *parent = 0);

		QObject* ParentMultiTabs ();
		TabClassInfo GetTabClassInfo () const;
		QToolBar* GetToolBar () const;
		void Remove ();


		static void SetParentMultiTabs (QObject *tab);
	private:
		void SetTextEditor ();
		void SetToolBarActions ();
		void SetDefaultSideWidgets ();
		void RemovePostingTargetsWidget ();
		void FillWidget (const Entry& e, const QByteArray& accId = QByteArray ());

		void ClearEntry ();

		Entry GetCurrentEntry ();

	private slots:
		void handleCurrentAccountChanged (int id);
		void fillCurrentTabWithEntry (const Entry& entry);
		void fillNewTabWithEntry (const Entry& entry, const QByteArray& accountId);


		void newEntry ();
		void saveEntry ();
		void saveNewEntry ();
		void submit (const Entry& e = Entry ());
		void on_SideWidget__dockLocationChanged (Qt::DockWidgetArea area);
		void on_UpdateProfile__triggered ();
		void handleOpenEntryInCurrentTab (const QModelIndex& index = QModelIndex ());
		void handleOpenEntryInNewTab (const QModelIndex& index = QModelIndex ());
		void handleOpenDraftInCurrentTab (const QModelIndex& index = QModelIndex ());
		void handleOpenDraftInNewTab (const QModelIndex& index = QModelIndex ());

		void loadPostsByDate (const QDate& date);

		void handleStorageUpdated ();

		void loadLocalEntries ();
		void handleLoadEntries (const QList<Entry>& entries);

	signals:
		void removeTab (QWidget *tab);
		void addNewTab (const QString& name, QWidget *tab);
	};
}
}
