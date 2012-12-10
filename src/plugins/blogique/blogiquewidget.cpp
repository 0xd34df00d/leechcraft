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

#include "blogiquewidget.h"
#include <stdexcept>
#include <QWidgetAction>
#include <QComboBox>
#include <QInputDialog>
#include <QStandardItemModel>
#include <QMessageBox>
#include <util/util.h>
#include <interfaces/itexteditor.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/imwproxy.h>
#include "interfaces/blogique/ibloggingplatform.h"
#include "interfaces/blogique/iblogiquesidewidget.h"
#include "interfaces/blogique/iprofile.h"
#include "interfaces/blogique/ipostoptionswidget.h"
#include "blogique.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "localstorage.h"
#include "updateentriesdialog.h"

namespace LeechCraft
{
namespace Blogique
{
	QObject *BlogiqueWidget::S_ParentMultiTabs_ = 0;

	BlogiqueWidget::BlogiqueWidget (QWidget *parent)
	: QWidget (parent)
	, PostEdit_ (0)
	, PostEditWidget_ (0)
	, ToolBar_ (new QToolBar)
	, PostTargetAction_ (0)
	, PrevAccountId_ (0)
	, PostsViewModel_ (new QStandardItemModel (this))
	, DraftsViewModel_ (new QStandardItemModel (this))
	, Storage_ (Core::Instance ().GetStorage ())
	, OpenDraftInNewTab_ (new QAction (tr ("Open in new tab"), this))
	, OpenDraftInCurrentTab_ (new QAction (tr ("Open here"), this))
	, OpenEntryInNewTab_ (new QAction (tr ("Open in new tab"), this))
	, OpenEntryInCurrentTab_ (new QAction (tr ("Open here"), this))
	, LoadLocalEntries_ (new QAction (tr ("Local entries"), this))
	, DraftID_ (-1)
	, EventID_ (-1)
	{
		Ui_.setupUi (this);

		auto mw = Core::Instance ().GetCoreProxy ()->GetMWProxy ();
		mw->AddDockWidget (Qt::RightDockWidgetArea, Ui_.SideWidget_);
		mw->AssociateDockWidget (Ui_.SideWidget_, this);
		mw->ToggleViewActionVisiblity (Ui_.SideWidget_, false);

		SetTextEditor ();

		SetToolBarActions ();

		SetDeafultSideWidgets ();

		const int accCount = Core::Instance ().GetAccounts ().count ();
		if (accCount == 1)
			AccountsBox_->setCurrentIndex (accCount);

		connect (this,
				SIGNAL (addNewTab (QString, QWidget*)),
				&Core::Instance (),
				SIGNAL (addNewTab (QString, QWidget*)));

		connect (&Core::Instance (),
				SIGNAL (gotEntries (QList<Entry>)),
				this,
				SLOT (handleGotEntries (QList<Entry>)));
		connect (&Core::Instance (),
				SIGNAL (storageUpdated ()),
				this,
				SLOT (handleStorageUpdated ()));
	}

	QObject* BlogiqueWidget::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	TabClassInfo BlogiqueWidget::GetTabClassInfo () const
	{
		return qobject_cast<Plugin*> (S_ParentMultiTabs_)->
				GetTabClasses ().first ();
	}

	QToolBar* BlogiqueWidget::GetToolBar () const
	{
		return ToolBar_;
	}

	void BlogiqueWidget::Remove ()
	{
		emit removeTab (this);
		PostTargetBox_->deleteLater ();
		Ui_.SideWidget_->deleteLater ();
		deleteLater ();
	}

	void BlogiqueWidget::FillWidget (const Entry& e, bool isDraft, const QByteArray& accId)
	{
		for (int i = 0; !accId.isEmpty () && i < AccountsBox_->count (); ++i)
		{
			if (Id2Account_.contains (i) &&
					Id2Account_ [i]->GetAccountID () == accId)
			{
				AccountsBox_->setCurrentIndex (i);
				break;
			}
		}

		Ui_.Subject_->setText (e.Subject_);
		PostEdit_->SetContents (e.Content_, ContentType::HTML);

		for (auto w : SidePluginsWidgets_)
		{
			auto ibsw = qobject_cast<IBlogiqueSideWidget*> (w);
			if (!ibsw)
				continue;

			switch (ibsw->GetWidgetType ())
			{
				case SideWidgetType::PostOptionsSideWidget:
				{
					ibsw->SetPostOptions (e.PostOptions_);
					auto ipow = qobject_cast<IPostOptionsWidget*> (w);
					if (!ipow)
						break;

					ipow->SetTags (e.Tags_);
					ipow->SetPostDate (e.Date_);
					break;
				}
				case SideWidgetType::CustomSideWidget:
					ibsw->SetCustomData (e.CustomData_);
					break;
				default:
					break;
			}
		}

		if (isDraft)
		{
			DraftID_ = e.EntryId_;
			EventID_ = -1;
		}
		else
		{
			EventID_ = e.EntryId_;
			DraftID_ = -1;
		}
	}

	void BlogiqueWidget::SetParentMultiTabs (QObject *tab)
	{
		S_ParentMultiTabs_ = tab;
	}

	void BlogiqueWidget::SetTextEditor ()
	{
		auto plugs = Core::Instance ().GetCoreProxy ()->
				GetPluginsManager ()->GetAllCastableTo<ITextEditor*> ();

		QVBoxLayout *editFrameLay = new QVBoxLayout ();
		editFrameLay->setContentsMargins (0, 0, 0, 0);
		Ui_.PostFrame_->setLayout (editFrameLay);

		Q_FOREACH (ITextEditor *plug, plugs)
		{
			if (!plug->SupportsEditor (ContentType::PlainText))
				continue;

			QWidget *w = plug->GetTextEditor (ContentType::PlainText);
			PostEdit_ = qobject_cast<IEditorWidget*> (w);
			if (!PostEdit_)
			{
				delete w;
				continue;
			}

			PostEditWidget_ = w;
			editFrameLay->addWidget (w);
			break;
		}
	}

	void BlogiqueWidget::SetToolBarActions ()
	{
		Ui_.NewEntry_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("document-new"));
		ToolBar_->addAction (Ui_.NewEntry_);
		connect (Ui_.NewEntry_,
				SIGNAL (triggered ()),
				this,
				SLOT (newEntry ()));

		Ui_.SaveEntry_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("document-save"));
		ToolBar_->addAction (Ui_.SaveEntry_);
		connect (Ui_.SaveEntry_,
				SIGNAL (triggered ()),
				this,
				SLOT (saveEntry ()));

		Ui_.SaveNewEntry_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("document-save-as"));
		ToolBar_->addAction (Ui_.SaveNewEntry_);
		connect (Ui_.SaveNewEntry_,
				SIGNAL (triggered ()),
				this,
				SLOT (saveNewEntry ()));

		Ui_.Submit_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("svn-commit"));
		ToolBar_->addAction (Ui_.Submit_);
		connect (Ui_.Submit_,
				SIGNAL (triggered ()),
				this,
				SLOT (submit ()));

		Ui_.OpenInBrowser_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("applications-internet"));
		ToolBar_->addAction (Ui_.OpenInBrowser_);

		Ui_.UpdateProfile_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("view-refresh"));

		ToolBar_->addSeparator ();

		AccountsBox_ = new QComboBox ();
		AccountsBox_->addItem (QString ());
		connect (AccountsBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleCurrentAccountChanged (int)));

		for (IAccount *acc : Core::Instance ().GetAccounts ())
		{
			AccountsBox_->addItem (acc->GetAccountName ());
			Id2Account_ [AccountsBox_->count () - 1] = acc;
		}

		ToolBar_->addWidget (AccountsBox_);

		PostTargetBox_ = new QComboBox;
	}

	void BlogiqueWidget::SetDeafultSideWidgets ()
	{
		if (!Ui_.CalendarSplitter_->restoreState (XmlSettingsManager::Instance ()
				.property ("CalendarSplitterPosition").toByteArray ()))
		{
			Ui_.CalendarSplitter_->setStretchFactor (0, 1);
			Ui_.CalendarSplitter_->setStretchFactor (1, 4);
		}

		connect (Ui_.CalendarSplitter_,
				SIGNAL (splitterMoved (int, int)),
				this,
				SLOT (saveSplitterPosition (int, int)));

		connect (Ui_.Calendar_,
				SIGNAL (activated (QDate)),
				this,
				SLOT (loadPostsByDate (QDate)));

		DraftsViewModel_->setHorizontalHeaderLabels ({ tr ("Date"), tr ("Name") });
		Ui_.LocalEntriesView_->setModel (DraftsViewModel_);
		connect (OpenDraftInCurrentTab_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenDraftInCurrentTab ()));
		connect (OpenDraftInNewTab_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenDraftInNewTab ()));
		Ui_.LocalEntriesView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		Ui_.LocalEntriesView_->addActions ({ OpenDraftInNewTab_, OpenDraftInCurrentTab_ });

		PostsViewModel_->setHorizontalHeaderLabels ({ tr ("Date"), tr ("Name") });
		Ui_.PostsView_->setModel (PostsViewModel_);

		connect (OpenEntryInCurrentTab_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenEntryInCurrentTab ()));
		connect (OpenEntryInNewTab_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenEntryInNewTab ()));
		Ui_.PostsView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		Ui_.PostsView_->addActions ({ OpenEntryInNewTab_, OpenEntryInCurrentTab_ });

		Ui_.UpdateEntries_->addAction (LoadLocalEntries_);
		connect (LoadLocalEntries_,
				SIGNAL (triggered ()),
				this,
				SLOT (loadLocalEntries ()));
	}

	void BlogiqueWidget::RemovePostingTargetsWidget ()
	{
		if (PostTargetAction_->isVisible ())
		{
			PostTargetAction_->setVisible (false);
			PostTargetBox_->clear ();
		}
	}

	void BlogiqueWidget::FillPostingStatistic ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		QMap<QDate, int> statistic;
		try
		{
			statistic = Storage_->GetEntriesCountByDate (acc->GetAccountID ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		Ui_.Calendar_->SetStatistic (statistic);
	}

	void BlogiqueWidget::ClearEntry ()
	{
		Ui_.Subject_->clear ();
		PostEdit_->SetContents (QString (), ContentType::PlainText);
		for (auto w : SidePluginsWidgets_)
		{
			auto ibsw = qobject_cast<IBlogiqueSideWidget*> (w);
			if (!ibsw)
				continue;

			switch (ibsw->GetWidgetType ())
			{
				case SideWidgetType::PostOptionsSideWidget:
				{
					ibsw->SetPostOptions (QVariantMap ());
					auto ipow = qobject_cast<IPostOptionsWidget*> (w);
					if (!ipow)
						continue;

					ipow->SetPostDate (QDateTime::currentDateTime ());
					ipow->SetTags (QStringList ());
					break;
				}
				case SideWidgetType::CustomSideWidget:
					ibsw->SetCustomData (QVariantMap ());
					break;
				default:
					break;
			}
		}
	}

	QList<QStandardItem*> BlogiqueWidget::CreateItemsToView (const Entry& entry) const
	{
		QStandardItem *dateItem = new QStandardItem (entry.Date_
				.toString ("dd-MM-yyyy hh:mm"));
		dateItem->setData (entry.EntryId_, EntryIdRole::DBIdRole);
		dateItem->setEditable (false);
		dateItem->setData (entry.Subject_, Qt::ToolTipRole);
		QStandardItem *itemSubj = new QStandardItem (entry.Subject_);
		itemSubj->setEditable (false);
		itemSubj->setData (entry.Subject_, Qt::ToolTipRole);

		return { dateItem, itemSubj };
	}

	Entry BlogiqueWidget::GetCurrentEntry ()
	{
		if (!AccountsBox_->currentIndex ())
			return Entry ();

		const QString& content = PostEdit_->GetContents (ContentType::HTML);
		if (content.isEmpty ())
		{
			QMessageBox::warning (this,
					tr ("LeechCraft"),
					tr ("Entry can't be emprty."));
			return Entry ();
		}

		QVariantMap postOptions, customData;
		QDateTime dt;
		QStringList tags;
		for (auto w : SidePluginsWidgets_)
		{
			auto ibsw = qobject_cast<IBlogiqueSideWidget*> (w);
			if (!ibsw)
				continue;

			switch (ibsw->GetWidgetType ())
			{
				case SideWidgetType::PostOptionsSideWidget:
				{
					postOptions.unite (ibsw->GetPostOptions ());
					auto ipow = qobject_cast<IPostOptionsWidget*> (w);
					if (!ipow)
						continue;

					if (dt.isNull ())
						dt = ipow->GetPostDate ();
					if (tags.isEmpty ())
						tags = ipow->GetTags ();
					break;
				}
				case SideWidgetType::CustomSideWidget:
					customData.unite (ibsw->GetCustomData ());
					break;
				default:
					break;
			}
		}

		Entry e;
		e.Target_ = PostTargetBox_->currentText ();
		e.Content_ = content;
		e.Subject_ = Ui_.Subject_->text ();
		e.Date_ = dt;
		e.Tags_ = tags;
		e.PostOptions_ = postOptions;
		e.CustomData_ = customData;

		if (EventID_ > 0)
			e.EntryId_ = EventID_;
		else if (DraftID_ > 0)
			e.EntryId_ = DraftID_;

		return e;
	}

	void BlogiqueWidget::LoadDrafts ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		QList<Entry> entries;
		try
		{
			entries = Storage_->GetShortDrafts (acc->GetAccountID ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching short drafts"
					<< e.what ();
		}

		FillDraftsView (entries);
	}

	Entry BlogiqueWidget::LoadFullDraft (qlonglong draftID)
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return Entry ();

		try
		{
			return Storage_->GetFullDraft (acc->GetAccountID (), draftID);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching full draft"
					<< e.what ();
			return Entry ();
		}
	}

	void BlogiqueWidget::RemoveDraft (qlonglong draftId)
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		try
		{
			Storage_->RemoveDraft (acc->GetAccountID (), draftId);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error removing draft"
					<< e.what ();
		}
	}

	void BlogiqueWidget::LoadEntries ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		QList<Entry> entries;
		try
		{
			entries = Storage_->GetLastEntries (acc->GetAccountID (),
					XmlSettingsManager::Instance ()
						.Property ("LastLocalEntriesToView", 20).toInt ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		FillPostsView (entries);
		FillPostingStatistic ();
	}

	Entry BlogiqueWidget::LoadEntry (qlonglong entryId)
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return Entry ();

		try
		{
			return Storage_->GetEntry (acc->GetAccountID (), entryId);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching event"
					<< e.what ();
			return Entry ();
		}
	}

	void BlogiqueWidget::FillPostsView (const QList<Entry> entries)
	{
		PostsViewModel_->removeRows (0, PostsViewModel_->rowCount ());
		for (const auto& entry : entries)
		{
			const auto& items = CreateItemsToView (entry);
			if (items.isEmpty ())
				continue;

			PostsViewModel_->appendRow (items);
			PostItem2Entry_ [items.first ()] = entry;
		}
		Ui_.PostsView_->resizeColumnToContents (0);
	}

	void BlogiqueWidget::FillDraftsView (const QList<Entry> entries)
	{
		DraftsViewModel_->removeRows (0, DraftsViewModel_->rowCount ());
		for (const auto& entry : entries)
		{
			const auto& items = CreateItemsToView (entry);
			if (items.isEmpty ())
				continue;

			DraftsViewModel_->appendRow (items);
			DraftItem2Entry_ [items.first ()] = entry;
		}
		Ui_.LocalEntriesView_->resizeColumnToContents (0);
	}

	void BlogiqueWidget::handleCurrentAccountChanged (int id)
	{
		if (PrevAccountId_)
		{
			auto ibp = qobject_cast<IBloggingPlatform*> (Id2Account_ [PrevAccountId_]->
					GetParentBloggingPlatform ());
			for (auto action : ibp->GetEditorActions ())
				PostEdit_->RemoveAction (action);

			for (auto w : SidePluginsWidgets_)
				w->deleteLater ();
			SidePluginsWidgets_.clear ();

			RemovePostingTargetsWidget ();

			for (auto act : LoadActions_)
				Ui_.UpdateEntries_->removeAction (act);
			LoadActions_.clear ();
		}

		if (!id)
		{
			ToolBar_->removeAction (Ui_.UpdateProfile_);
			RemovePostingTargetsWidget ();
			DraftsViewModel_->removeRows (0, DraftsViewModel_->rowCount ());
			PostsViewModel_->removeRows (0, PostsViewModel_->rowCount ());
			return;
		}

		ToolBar_->insertAction (Ui_.OpenInBrowser_, Ui_.UpdateProfile_);


		auto account = Id2Account_ [id];
		auto actions = account->GetUpdateActions ();
		Ui_.UpdateEntries_->insertActions (LoadLocalEntries_, actions);
		LoadActions_ = actions;

		auto ibp = qobject_cast<IBloggingPlatform*> (account->
				GetParentBloggingPlatform ());

		if (ibp->GetFeatures () & IBloggingPlatform::BPFSelectablePostDestination)
		{
			if (!PostTargetAction_)
				PostTargetAction_ = ToolBar_->addWidget (PostTargetBox_);
			else
				PostTargetAction_->setVisible (true);

			IProfile *profile = qobject_cast<IProfile*> (account->GetProfile ());
			if (profile)
			{
				for (const auto& target : profile->GetPostingTargets ())
					PostTargetBox_->addItem (target.first, target.second);
			}
		}

		for (auto action : ibp->GetEditorActions ())
			PostEdit_->AppendAction (action);

		for (auto w : ibp->GetBlogiqueSideWidgets ())
		{
			IBlogiqueSideWidget *ibsw = qobject_cast<IBlogiqueSideWidget*> (w);
			if (!ibsw)
			{
				qWarning () << Q_FUNC_INFO
						<< "Side widget"
						<< w
						<< "from"
						<< ibp
						<< "is not an IBlogiqueSideWidget";
				continue;
			}

			SidePluginsWidgets_ << w;
			ibsw->SetAccount (account->GetObject ());
			Ui_.Tools_->addItem (w, ibsw->GetName ());
		}

		LoadDrafts ();
		LoadEntries ();

		PrevAccountId_ = id;
	}

	void BlogiqueWidget::newEntry ()
	{
		//TODO ask about save.
		ClearEntry ();
	}

	void BlogiqueWidget::saveEntry ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		const Entry& e = GetCurrentEntry ();
		if (!e.IsEmpty ())
			try
			{
				if (DraftID_ == -1)
					DraftID_ = Storage_->SaveDraft (acc->GetAccountID (), e);
				else
					Storage_->UpdateDraft (DraftID_, e);
			}
			catch (const std::runtime_error& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error saving draft"
						<< e.what ();
			}

		LoadDrafts ();
	}

	void BlogiqueWidget::saveNewEntry ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		const Entry& e = GetCurrentEntry ();
		if (!e.IsEmpty ())
			try
			{
				DraftID_ = Storage_->SaveDraft (acc->GetAccountID (), e);
			}
			catch (const std::runtime_error& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error saving draft"
						<< e.what ();
			}

		LoadDrafts ();
	}

	void BlogiqueWidget::submit (const Entry& event)
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		const auto& e = event.IsEmpty () ?
			GetCurrentEntry () :
			event;

		if (!e.IsEmpty ())
			if (EventID_ > 0)
			{
				QMessageBox mbox (QMessageBox::Question,
						"LeechCraft",
						tr ("Do you want to update entry or to post new?"),
						QMessageBox::Yes | QMessageBox::Cancel,
						this);
				mbox.setDefaultButton (QMessageBox::Cancel);
				mbox.setButtonText (QMessageBox::Yes, tr ("Update post"));
				QPushButton newPostButton (tr ("Post new"));
				mbox.addButton (&newPostButton, QMessageBox::AcceptRole);

				if (mbox.exec () == QMessageBox::Cancel)
					return;
				else if (mbox.clickedButton () == &newPostButton)
					acc->submit (e);
				else
					acc->UpdateEntry (e);
			}
			else
				acc->submit (e);
	}

	void BlogiqueWidget::saveSplitterPosition (int, int)
	{
		XmlSettingsManager::Instance ().setProperty ("CalendarSplitterPosition",
				Ui_.CalendarSplitter_->saveState ());
	}

	void BlogiqueWidget::on_UpdateProfile__triggered ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		acc->updateProfile ();
	}

	void BlogiqueWidget::on_RemoveDraft__released ()
	{
		QModelIndex idx = Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		if (XmlSettingsManager::Instance ()
				.Property ("ConfirmDraftRemoving", true).toBool ())
		{
			QMessageBox mbox (QMessageBox::Question,
					"LeechCraft",
					tr ("Do you want to delete selected draft?"),
					QMessageBox::Yes | QMessageBox::No,
					this);
			mbox.setDefaultButton (QMessageBox::No);

			QPushButton always (tr ("Always"));
			mbox.addButton (&always, QMessageBox::AcceptRole);

			if (mbox.exec () == QMessageBox::No)
				return;
			else if (mbox.clickedButton () == &always)
				XmlSettingsManager::Instance ()
						.setProperty ("ConfirmDraftRemoving", false);
		}

		idx = idx.sibling (idx.row (), Columns::Date);
		RemoveDraft (idx.data (EntryIdRole::DBIdRole).toLongLong ());
		DraftsViewModel_->removeRow (idx.row ());
	}

	void BlogiqueWidget::on_PublishDraft__released ()
	{
		QModelIndex idx = Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		idx = idx.sibling (idx.row (), Columns::Date);
		const Entry& e = LoadFullDraft (idx.data (EntryIdRole::DBIdRole).toLongLong ());
		if (e.IsEmpty ())
			return;

		submit (e);
	}

	void BlogiqueWidget::on_RemoveRemotePost__released ()
	{
		if (!Ui_.PostsView_->currentIndex ().isValid ())
			return;

		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		auto idx = Ui_.PostsView_->currentIndex ();
		idx = idx.sibling (idx.row (), Columns::Date);
		const Entry& e = LoadEntry (idx.data (EntryIdRole::DBIdRole).toLongLong ());
		if (e.IsEmpty ())
			return;

		if (QMessageBox::question (this, "LeechCraft",
				tr ("Do you want to remove this entry from blog?"),
				QMessageBox::Ok | QMessageBox::Cancel,
				QMessageBox::Cancel) == QMessageBox::Ok)
			acc->RemoveEntry (e);
	}

	void BlogiqueWidget::on_Edit__released ()
	{
		const QModelIndex& currentIndex = Ui_.PostsView_->currentIndex ();
		if (!currentIndex.isValid ())
			return;

		handleOpenEntryInCurrentTab (currentIndex);
	}

	void BlogiqueWidget::on_PostsView__doubleClicked (const QModelIndex& index)
	{
		XmlSettingsManager::Instance ()
				.property ("OpenEntryByDblClick").toString () == "CurrentTab" ?
			handleOpenEntryInCurrentTab (index) :
			handleOpenEntryInNewTab (index);
	}

	void BlogiqueWidget::handleOpenEntryInCurrentTab (const QModelIndex& index)
	{
		QModelIndex idx = index.isValid () ?
				index :
				Ui_.PostsView_->currentIndex ();
		if (!idx.isValid ())
			return;

		idx = idx.sibling (idx.row (), Columns::Date);
		const Entry& e = LoadEntry (idx.data (EntryIdRole::DBIdRole).toLongLong ());
		FillWidget (e);
	}

	void BlogiqueWidget::handleOpenEntryInNewTab (const QModelIndex& index)
	{
		QModelIndex idx = index.isValid () ?
				index :
				Ui_.PostsView_->currentIndex ();
		if (!idx.isValid ())
			return;

		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		idx = idx.sibling (idx.row (), Columns::Date);
		const Entry& e = LoadEntry (idx.data (EntryIdRole::DBIdRole).toLongLong ());

		auto newTab = new BlogiqueWidget;
		connect (newTab,
				 SIGNAL (removeTab (QWidget*)),
				 &Core::Instance (),
				 SIGNAL (removeTab (QWidget*)));

		newTab->FillWidget (e, false, acc->GetAccountID ());
		emit addNewTab ("Blogique", newTab);
	}

	void BlogiqueWidget::on_LocalEntriesView__doubleClicked (const QModelIndex& index)
	{
		XmlSettingsManager::Instance ()
				.property ("OpenDraftByDblClick").toString () == "CurrentTab" ?
			handleOpenDraftInCurrentTab (index) :
			handleOpenDraftInNewTab (index);
	}

	void BlogiqueWidget::handleOpenDraftInCurrentTab (const QModelIndex& index)
	{
		QModelIndex idx = index.isValid () ?
			index :
			Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		idx = idx.sibling (idx.row (), Columns::Date);
		const Entry& e = LoadFullDraft (idx.data (EntryIdRole::DBIdRole).toLongLong ());
		FillWidget (e, true);
	}

	void BlogiqueWidget::handleOpenDraftInNewTab (const QModelIndex& index)
	{
		QModelIndex idx = index.isValid () ?
				index :
				Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		idx = idx.sibling (idx.row (), Columns::Date);
		const Entry& e = LoadFullDraft (idx.data (EntryIdRole::DBIdRole).toLongLong ());

		auto newTab = new BlogiqueWidget;
		connect (newTab,
				SIGNAL (removeTab (QWidget*)),
				&Core::Instance (),
				SIGNAL (removeTab (QWidget*)));

		newTab->FillWidget (e, true, acc->GetAccountID ());
		emit addNewTab ("Blogique", newTab);
	}

	void BlogiqueWidget::loadPostsByDate (const QDate& date)
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		QList<Entry> entries;
		try
		{
			entries = Storage_->GetEntriesByDate (acc->GetAccountID (), date);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		FillPostsView (entries);
	}

	void BlogiqueWidget::handleGotEntries (const QList<Entry>& entries)
	{
		handleLoadEntries (entries);
		LoadDrafts ();
	}

	void BlogiqueWidget::handleStorageUpdated ()
	{
		LoadDrafts ();
	}

	void BlogiqueWidget::loadLocalEntries ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		int count = XmlSettingsManager::Instance ()
				.Property ("LastLocalEntriesToView", 20).toInt ();
		if (XmlSettingsManager::Instance ().Property ("LocalLoadAsk", true).toBool ())
		{
			UpdateEntriesDialog dlg;
			if (dlg.exec () == QDialog::Rejected)
				return;
			count = dlg.GetCount ();
		}

		QList<Entry> entries;
		try
		{
			entries = Storage_->GetLastEntries (acc->GetAccountID (), count);
		}
		catch (const std::runtime_error& err)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< err.what ();
		}
		FillPostsView (entries);
	}

	void BlogiqueWidget::handleLoadEntries (const QList<Entry>& entries)
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		FillPostsView (entries);
		FillPostingStatistic ();
	}

}
}

