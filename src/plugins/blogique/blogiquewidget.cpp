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
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QWidgetAction>
#include <util/util.h>
#include <interfaces/itexteditor.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
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
#include "localentrieswidget.h"
#include "remoteentrieswidget.h"

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
	, LocalEntriesWidget_ (new LocalEntriesWidget)
	, RemoteEntriesWidget_ (new RemoteEntriesWidget)
	, PrevAccountId_ (-1)
	{
		Ui_.setupUi (this);

		auto dwa = static_cast<Qt::DockWidgetArea> (XmlSettingsManager::Instance ()
				.Property ("DockWidgetArea", Qt::RightDockWidgetArea).toInt ());
		if (dwa == Qt::NoDockWidgetArea)
			dwa = Qt::RightDockWidgetArea;
		auto mw = Core::Instance ().GetCoreProxy ()->GetRootWindowsManager ()->GetMWProxy (0);
		mw->AddDockWidget (dwa, Ui_.SideWidget_);
		mw->AssociateDockWidget (Ui_.SideWidget_, this);
		mw->ToggleViewActionVisiblity (Ui_.SideWidget_, false);

		SetTextEditor ();

		SetDefaultSideWidgets ();

		SetToolBarActions ();

		connect (this,
				SIGNAL (addNewTab (QString, QWidget*)),
				&Core::Instance (),
				SIGNAL (addNewTab (QString, QWidget*)));

		connect (&Core::Instance (),
				SIGNAL (storageUpdated ()),
				this,
				SLOT (handleStorageUpdated ()));

		connect (RemoteEntriesWidget_,
				SIGNAL(fillCurrentWidgetWithRemoteEntry (Entry)),
				this,
				SLOT (fillCurrentTabWithEntry (Entry)));
		connect (RemoteEntriesWidget_,
				SIGNAL(fillNewWidgetWithRemoteEntry (Entry,QByteArray)),
				this,
				SLOT (fillNewTabWithEntry (Entry, QByteArray)));
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

	void BlogiqueWidget::FillWidget (const Entry& e, const QByteArray& accId)
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
		Ui_.NewEntry_->setProperty ("ActionIcon", "document-new");
		ToolBar_->addAction (Ui_.NewEntry_);
		connect (Ui_.NewEntry_,
				SIGNAL (triggered ()),
				this,
				SLOT (newEntry ()));

		Ui_.SaveEntry_->setProperty ("ActionIcon", "document-save");
		ToolBar_->addAction (Ui_.SaveEntry_);
		connect (Ui_.SaveEntry_,
				SIGNAL (triggered ()),
				this,
				SLOT (saveEntry ()));

		Ui_.SaveNewEntry_->setProperty ("ActionIcon", "document-save-as");
		ToolBar_->addAction (Ui_.SaveNewEntry_);
		connect (Ui_.SaveNewEntry_,
				SIGNAL (triggered ()),
				this,
				SLOT (saveNewEntry ()));

		Ui_.Submit_->setProperty ("ActionIcon", "svn-commit");
		ToolBar_->addAction (Ui_.Submit_);
		connect (Ui_.Submit_,
				SIGNAL (triggered ()),
				this,
				SLOT (submit ()));

		Ui_.OpenInBrowser_->setProperty ("ActionIcon", "applications-internet");
		ToolBar_->addAction (Ui_.OpenInBrowser_);

		Ui_.UpdateProfile_->setProperty ("ActionIcon", "view-refresh");

		ToolBar_->addSeparator ();

		QList<QAction*> editorActions;
		if (PostEdit_)
		{
			editorActions << PostEdit_->GetEditorAction (EditorAction::Find);
			editorActions << PostEdit_->GetEditorAction (EditorAction::Replace);
			editorActions.removeAll (0);
		}
		if (!editorActions.isEmpty ())
		{
			PostEdit_->AppendSeparator ();
			for (auto action : editorActions)
				PostEdit_->AppendAction (action);
			PostEdit_->AppendSeparator ();
		}

		AccountsBox_ = new QComboBox ();
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

		int index = AccountsBox_->findText (XmlSettingsManager::Instance ()
				.property ("LastActiveAccountName").toString (),
					Qt::MatchFixedString);
		handleCurrentAccountChanged (index == -1 ? 0 : index);
	}

	void BlogiqueWidget::SetDefaultSideWidgets ()
	{
		for (int i = 0; i < Ui_.Tools_->count (); ++i)
		{
			auto w = Ui_.Tools_->widget (i);
			w->deleteLater ();
		}
		Ui_.Tools_->addItem (LocalEntriesWidget_, LocalEntriesWidget_->GetName ());
	}

	void BlogiqueWidget::RemovePostingTargetsWidget ()
	{
		if (PostTargetAction_->isVisible ())
		{
			PostTargetAction_->setVisible (false);
			PostTargetBox_->clear ();
		}
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

	Entry BlogiqueWidget::GetCurrentEntry ()
	{
		if (!AccountsBox_->count ())
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
/*
		if (EntryID_ > 0)
			e.EntryId_ = EntryID_;
		else if (DraftID_ > 0)
			e.EntryId_ = DraftID_;*/

		return e;
	}

	void BlogiqueWidget::handleCurrentAccountChanged (int id)
	{
		if (Id2Account_.isEmpty ())
			return;

		if (PrevAccountId_ != -1)
		{
			auto ibp = qobject_cast<IBloggingPlatform*> (Id2Account_ [PrevAccountId_]->
					GetParentBloggingPlatform ());
			for (auto action : ibp->GetEditorActions ())
				PostEdit_->RemoveAction (action);

			for (auto w : SidePluginsWidgets_)
				w->deleteLater ();
			SidePluginsWidgets_.clear ();

			RemovePostingTargetsWidget ();
		}

		ToolBar_->insertAction (Ui_.OpenInBrowser_, Ui_.UpdateProfile_);

		auto account = Id2Account_ [id];
		LocalEntriesWidget_->SetAccount (account);
		RemoteEntriesWidget_->SetAccount (account);

		auto ibp = qobject_cast<IBloggingPlatform*> (account->
				GetParentBloggingPlatform ());

		if (!(ibp->GetFeatures () & IBloggingPlatform::BPFLocalBlog))
		{
			if (Ui_.Tools_->indexOf (RemoteEntriesWidget_) == -1)
				Ui_.Tools_->addItem (RemoteEntriesWidget_,
						RemoteEntriesWidget_->GetName ());
			else
				RemoteEntriesWidget_->clear ();
		}
		else
		{
			int index = Ui_.Tools_->indexOf (RemoteEntriesWidget_);
			if (index != -1)
				Ui_.Tools_->removeItem (index);
			LocalEntriesWidget_->clear ();
		}

		if (ibp->GetFeatures () & IBloggingPlatform::BPFSelectablePostDestination)
		{
			if (!PostTargetAction_)
				PostTargetAction_ = ToolBar_->addWidget (PostTargetBox_);
			else
				PostTargetAction_->setVisible (true);

			IProfile *profile = qobject_cast<IProfile*> (account->GetProfile ());
			if (profile)
				for (const auto& target : profile->GetPostingTargets ())
					PostTargetBox_->addItem (target.first, target.second);
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

// 		LoadDrafts ();
// 		LoadEntries ();
//
		PrevAccountId_ = id;
	}

	void BlogiqueWidget::fillCurrentTabWithEntry (const Entry& entry)
	{
		FillWidget (entry);
	}

	void BlogiqueWidget::fillNewTabWithEntry (const Entry& entry,
			const QByteArray& accountId)
	{
		auto w = Core::Instance ().CreateBlogiqueWidget ();
		w->FillWidget (entry, accountId);
		emit addNewTab ("Blogique", w);
	}

	void BlogiqueWidget::newEntry ()
	{
// 		//TODO ask about save.
// 		DraftID_ = -1;
// 		EntryID_ = -1;
// 		ClearEntry ();
	}

	void BlogiqueWidget::saveEntry ()
	{
// 		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
// 		if (!acc)
// 			return;
//
// 		const Entry& e = GetCurrentEntry ();
// 		if (!e.IsEmpty ())
// 			try
// 			{
// 				if (DraftID_ == -1)
// 					DraftID_ = Storage_->SaveDraft (acc->GetAccountID (), e);
// 				else
// 					Storage_->UpdateDraft (DraftID_, e);
// 			}
// 			catch (const std::runtime_error& e)
// 			{
// 				qWarning () << Q_FUNC_INFO
// 						<< "error saving draft"
// 						<< e.what ();
// 			}
//
// 		LoadDrafts ();
	}

	void BlogiqueWidget::saveNewEntry ()
	{
// 		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
// 		if (!acc)
// 			return;
//
// 		const Entry& e = GetCurrentEntry ();
// 		if (!e.IsEmpty ())
// 			try
// 			{
// 				DraftID_ = Storage_->SaveDraft (acc->GetAccountID (), e);
// 			}
// 			catch (const std::runtime_error& e)
// 			{
// 				qWarning () << Q_FUNC_INFO
// 						<< "error saving draft"
// 						<< e.what ();
// 			}
//
// 		LoadDrafts ();
	}

	void BlogiqueWidget::submit (const Entry& event)
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		const auto& e = event.IsEmpty () ?
			GetCurrentEntry () :
			event;

// 		if (!e.IsEmpty ())
// 		{
// 			if (EntryID_ > 0)
// 			{
// 				QMessageBox mbox (QMessageBox::Question,
// 						"LeechCraft",
// 						tr ("Do you want to update entry or to post new one?"),
// 						QMessageBox::Yes | QMessageBox::Cancel,
// 						this);
// 				mbox.setDefaultButton (QMessageBox::Cancel);
// 				mbox.setButtonText (QMessageBox::Yes, tr ("Update post"));
// 				QPushButton newPostButton (tr ("Post new"));
// 				mbox.addButton (&newPostButton, QMessageBox::AcceptRole);
//
// 				if (mbox.exec () == QMessageBox::Cancel)
// 					return;
// 				else if (mbox.clickedButton () == &newPostButton)
// 					acc->submit (e);
// 				else
// 					acc->UpdateEntry (e);
// 			}
// 			else
// 				acc->submit (e);
// 		}
	}

	void BlogiqueWidget::on_SideWidget__dockLocationChanged (Qt::DockWidgetArea area)
	{
		if (area != Qt::AllDockWidgetAreas &&
				area != Qt::NoDockWidgetArea)
			XmlSettingsManager::Instance ().setProperty ("DockWidgetArea", area);
	}

	void BlogiqueWidget::on_UpdateProfile__triggered ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		acc->updateProfile ();
	}

// 	void BlogiqueWidget::on_PostsView__doubleClicked (const QModelIndex& index)
// 	{
// 		XmlSettingsManager::Instance ()
// 				.property ("OpenEntryByDblClick").toString () == "CurrentTab" ?
// 			handleOpenEntryInCurrentTab (index) :
// 			handleOpenEntryInNewTab (index);
// 	}

	void BlogiqueWidget::handleOpenEntryInCurrentTab (const QModelIndex& index)
	{
// 		QModelIndex idx = index.isValid () ?
// 				index :
// 				Ui_.PostsView_->currentIndex ();
// 		if (!idx.isValid ())
// 			return;
//
// 		idx = idx.sibling (idx.row (), Columns::Date);
// 		const Entry& e = LoadEntry (idx.data (EntryIdRole::DBIdRole).toLongLong ());
// 		FillWidget (e);
	}

	void BlogiqueWidget::handleOpenEntryInNewTab (const QModelIndex& index)
	{
// 		QModelIndex idx = index.isValid () ?
// 				index :
// 				Ui_.PostsView_->currentIndex ();
// 		if (!idx.isValid ())
// 			return;
//
// 		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
// 		if (!acc)
// 			return;
//
// 		idx = idx.sibling (idx.row (), Columns::Date);
// 		const Entry& e = LoadEntry (idx.data (EntryIdRole::DBIdRole).toLongLong ());
//
// 		auto newTab = Core::Instance ().CreateBlogiqueWidget ();
// 		newTab->FillWidget (e, false, acc->GetAccountID ());
// 		emit addNewTab ("Blogique", newTab);
	}

// 	void BlogiqueWidget::on_LocalEntriesView__doubleClicked (const QModelIndex& index)
// 	{
// 		XmlSettingsManager::Instance ()
// 				.property ("OpenDraftByDblClick").toString () == "CurrentTab" ?
// 			handleOpenDraftInCurrentTab (index) :
// 			handleOpenDraftInNewTab (index);
// 	}

	void BlogiqueWidget::handleOpenDraftInCurrentTab (const QModelIndex& index)
	{
// 		QModelIndex idx = index.isValid () ?
// 			index :
// 			Ui_.LocalEntriesView_->currentIndex ();
// 		if (!idx.isValid ())
// 			return;
//
// 		idx = idx.sibling (idx.row (), Columns::Date);
// 		const Entry& e = LoadFullDraft (idx.data (EntryIdRole::DBIdRole).toLongLong ());
// 		FillWidget (e, true);
	}

	void BlogiqueWidget::handleOpenDraftInNewTab (const QModelIndex& index)
	{
// 		QModelIndex idx = index.isValid () ?
// 				index :
// 				Ui_.LocalEntriesView_->currentIndex ();
// 		if (!idx.isValid ())
// 			return;
//
// 		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
// 		if (!acc)
// 			return;
//
// 		idx = idx.sibling (idx.row (), Columns::Date);
// 		const Entry& e = LoadFullDraft (idx.data (EntryIdRole::DBIdRole).toLongLong ());
//
// 		auto newTab = Core::Instance ().CreateBlogiqueWidget ();
// 		newTab->FillWidget (e, true, acc->GetAccountID ());
// 		emit addNewTab ("Blogique", newTab);
	}

	void BlogiqueWidget::loadPostsByDate (const QDate& date)
	{
// 		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
// 		if (!acc)
// 			return;
//
// 		QList<Entry> entries;
// 		try
// 		{
// 			entries = Storage_->GetEntriesByDate (acc->GetAccountID (), date);
// 		}
// 		catch (const std::runtime_error& e)
// 		{
// 			qWarning () << Q_FUNC_INFO
// 					<< "error fetching entries"
// 					<< e.what ();
// 		}
//
// 		FillPostsView (entries);
	}

	void BlogiqueWidget::handleStorageUpdated ()
	{
// 		LoadDrafts ();
	}

	void BlogiqueWidget::loadLocalEntries ()
	{
// 		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
// 		if (!acc)
// 			return;
//
// 		int count = XmlSettingsManager::Instance ()
// 				.Property ("LastLocalEntriesToView", 20).toInt ();
// 		if (XmlSettingsManager::Instance ().Property ("LocalLoadAsk", true).toBool ())
// 		{
// 			UpdateEntriesDialog dlg;
// 			if (dlg.exec () == QDialog::Rejected)
// 				return;
// 			count = dlg.GetCount ();
// 		}
//
// 		QList<Entry> entries;
// 		try
// 		{
// 			entries = Storage_->GetLastEntries (acc->GetAccountID (), count);
// 		}
// 		catch (const std::runtime_error& err)
// 		{
// 			qWarning () << Q_FUNC_INFO
// 					<< "error fetching entries"
// 					<< err.what ();
// 		}
// 		FillPostsView (entries);
	}

	void BlogiqueWidget::handleLoadEntries (const QList<Entry>& entries)
	{
// 		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
// 		if (!acc)
// 			return;
//
// 		FillPostsView (entries);
// 		FillPostingStatistic ();
	}

}
}

