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
#include <interfaces/ihaverecoverabletabs.h>
#include "interfaces/blogique/iaccount.h"
#include "ui_blogiquewidget.h"

class QStandardItem;
class QStandardItemModel;
class IEditorWidget;
class QToolBar;
class QComboBox;
class QProgressBar;

namespace LeechCraft
{
namespace Blogique
{
	class IBlogiqueSideWidget;
	class IAccount;
	class DraftEntriesWidget;
	class BlogEntriesWidget;

	class BlogiqueWidget : public QWidget
						, public ITabWidget
						, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		enum BlogiqueSideWidgets
		{
			PostOptionsWidget = 2
		};

		static QObject *S_ParentMultiTabs_;

		Ui::BlogiqueWidget Ui_;

		IEditorWidget *PostEdit_;
		QWidget *PostEditWidget_;
		QToolBar *ToolBar_;
		QToolBar *ProgressToolBar_;
		QAction *AccountsBoxAction_;
		QComboBox *AccountsBox_;
		QComboBox *PostTargetBox_;
		QAction *PostTargetAction_;
		QAction *ProgressBarLabelAction_;
		QLabel *ProgressBarLabel_;
		QAction *ProgressBarAction_;

		DraftEntriesWidget *DraftEntriesWidget_;
		BlogEntriesWidget *BlogEntriesWidget_;
		QHash<int, IAccount*> Id2Account_;
		int PrevAccountId_;
		QList<QWidget*> SidePluginsWidgets_;

		EntryType EntryType_;
		qint64 EntryId_;

		bool EntryChanged_;

	public:
		BlogiqueWidget (QWidget *parent = 0);

		QObject* ParentMultiTabs ();
		TabClassInfo GetTabClassInfo () const;
		QToolBar* GetToolBar () const;
		void Remove ();

		static void SetParentMultiTabs (QObject *tab);

		QByteArray GetTabRecoverData () const;
		QString GetTabRecoverName () const;
		QIcon GetTabRecoverIcon () const;
		void FillWidget (const Entry& e, const QByteArray& accId = QByteArray ());
	private:
		void SetTextEditor ();
		void SetToolBarActions ();
		void SetDefaultSideWidgets ();
		void RemovePostingTargetsWidget ();

		void ClearEntry ();

		Entry GetCurrentEntry () const;

		void ShowProgress (const QString& labelText = QString ());

	public slots:
		void handleAutoSave ();
		void handleEntryPosted ();
		void handleEntryRemoved ();
		void handleRequestEntriesBegin ();
		void handleRequestEntriesEnd ();

	private slots:
		void handleCurrentAccountChanged (int id);
		void fillCurrentTabWithEntry (const Entry& entry);
		void fillNewTabWithEntry (const Entry& entry, const QByteArray& accountId);

		void handleEntryChanged (const QString& str = QString ());
		void handleRemovingEntryBegin ();

		void newEntry ();
		void saveEntry (const Entry& e = Entry ());
		void saveNewEntry (const Entry& e = Entry ());
		void submit (const Entry& e = Entry ());
		void submitTo (const Entry& e = Entry ());
		void on_SideWidget__dockLocationChanged (Qt::DockWidgetArea area);
		void on_UpdateProfile__triggered ();

	signals:
		void removeTab (QWidget *tab);
		void addNewTab (const QString& name, QWidget *tab);
		void changeTabName (QWidget *content, const QString& name);

		void tabRecoverDataChanged ();
	};
}
}
