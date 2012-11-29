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
#include "interfaces/blogique/ibloggingplatform.h"
#include "interfaces/blogique/iblogiquesidewidget.h"
#include "interfaces/blogique/iprofile.h"
#include "interfaces/blogique/ipostoptionswidget.h"
#include "blogique.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "localstorage.h"

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
	, DraftID_ (-1)
	{
		Ui_.setupUi (this);

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

		Ui_.SaveEntry_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("document-save"));
		ToolBar_->addAction (Ui_.SaveEntry_);
		Ui_.SaveNewEntry_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("document-save-as"));
		ToolBar_->addAction (Ui_.SaveNewEntry_);
		Ui_.Submit_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("svn-commit"));
		ToolBar_->addAction (Ui_.Submit_);

		Ui_.OpenInBrowser_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("applications-internet"));
		ToolBar_->addAction (Ui_.OpenInBrowser_);

		Ui_.UpdateProfile_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("view-refresh"));

		ToolBar_->addSeparator ();

		AccountsBox_ = new QComboBox (ToolBar_);
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

		connect (Ui_.SaveEntry_,
				SIGNAL (triggered ()),
				this,
				SLOT (saveEntry ()));
		connect (Ui_.SaveNewEntry_,
				SIGNAL (triggered ()),
				this,
				SLOT (saveNewEntry ()));
		connect (Ui_.Submit_,
				SIGNAL (triggered ()),
				this,
				SLOT (submit ()));

		if (!Ui_.MainSplitter_->restoreState (XmlSettingsManager::Instance ()
				.property ("MainSplitterPosition").toByteArray ()))
		{
			Ui_.MainSplitter_->setStretchFactor (0, 6);
			Ui_.MainSplitter_->setStretchFactor (1, 1);
		}

		if (!Ui_.CalendarSplitter_->restoreState (XmlSettingsManager::Instance ()
				.property ("CalendarSplitterPosition").toByteArray ()))
		{
			Ui_.CalendarSplitter_->setStretchFactor (0, 1);
			Ui_.CalendarSplitter_->setStretchFactor (1, 4);
		}

		connect (Ui_.MainSplitter_,
				SIGNAL (splitterMoved (int, int)),
				this,
				SLOT (saveSplitterPosition (int, int)));
		connect (Ui_.CalendarSplitter_,
				SIGNAL (splitterMoved (int, int)),
				this,
				SLOT (saveSplitterPosition (int, int)));

		DraftsViewModel_->setHorizontalHeaderLabels ({ tr ("Date"), tr ("Name") });
		Ui_.LocalEntriesView_->setModel (DraftsViewModel_);

		connect (OpenDraftInCurrentTab_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenInCurrentTab ()));
		connect (OpenDraftInNewTab_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenInNewTab ()));
		Ui_.LocalEntriesView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		Ui_.LocalEntriesView_->addActions ({ OpenDraftInNewTab_, OpenDraftInCurrentTab_ });

		connect (this,
				SIGNAL (addNewTab (QString, QWidget*)),
				&Core::Instance (),
				SIGNAL (addNewTab (QString, QWidget*)));
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
		deleteLater ();
	}

	void BlogiqueWidget::FillWidget (const Event& e, const QByteArray& accId)
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
				{
					ibsw->SetCustomData (e.CustomData_);
					break;
				}
			}
		}
	}

	void BlogiqueWidget::SetParentMultiTabs (QObject *tab)
	{
		S_ParentMultiTabs_ = tab;
	}

	void BlogiqueWidget::RemovePostingTargetsWidget ()
	{
		if (PostTargetAction_->isVisible ())
		{
			PostTargetAction_->setVisible (false);
			PostTargetBox_->clear ();
		}
	}

	Event BlogiqueWidget::GetCurrentEvent ()
	{
		if (!AccountsBox_->currentIndex ())
			return Event ();

		const QString& content = PostEdit_->GetContents (ContentType::PlainText);
		if (content.isEmpty ())
		{
			QMessageBox::warning (this,
					tr ("LeechCraft"),
					tr ("Entry can't be emprty."));
			return Event ();
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
			}
		}

		Event e;
		e.Target_ = PostTargetBox_->currentText ();
		e.Content_ = PostEdit_->GetContents (ContentType::HTML);
		e.Subject_ = Ui_.Subject_->text ();
		e.Date_ = dt;
		e.Tags_ = tags;
		e.PostOptions_ = postOptions;
		e.CustomData_ = customData;

		return e;
	}

	void BlogiqueWidget::LoadDrafts ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		DraftsViewModel_->removeRows (0, DraftsViewModel_->rowCount ());
		QList<Event> entries;
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

		for (const auto& entry : entries)
		{
			QStandardItem *dateItem = new QStandardItem (entry.Date_
					.toString (Qt::DefaultLocaleShortDate));
			dateItem->setData (entry.EntryDBId_, EntryIdRole::DBIdRole);
			dateItem->setEditable (false);
			QStandardItem *itemSubj = new QStandardItem (entry.Subject_);
			itemSubj->setEditable (false);
			DraftsViewModel_->appendRow ({ dateItem, itemSubj });

			DraftItem2Event_ [dateItem] = entry;
		}
		Ui_.LocalEntriesView_->resizeColumnToContents (0);
	}

	Event BlogiqueWidget::LoadFullDraft (const QByteArray& id, qlonglong draftID)
	{
		Event event;
		try
		{
			return Storage_->GetFullDraft (id, draftID);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching full draft"
					<< e.what ();
			return Event ();
		}
	}

	void BlogiqueWidget::RemoveDraft (qlonglong id)
	{
		try
		{
			Storage_->RemoveDraft (id);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error removing draft"
					<< e.what ();
		}
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
		}

		if (!id)
		{
			ToolBar_->removeAction (Ui_.UpdateProfile_);
			RemovePostingTargetsWidget ();
			DraftsViewModel_->removeRows (0, DraftsViewModel_->rowCount ());
			return;
		}
		PrevAccountId_ = id;

		ToolBar_->insertAction (Ui_.OpenInBrowser_, Ui_.UpdateProfile_);

		auto ibp = qobject_cast<IBloggingPlatform*> (Id2Account_ [PrevAccountId_]->
				GetParentBloggingPlatform ());

		if (ibp->GetFeatures () & IBloggingPlatform::BPFSelectablePostDestination)
		{
			if (!PostTargetAction_)
				PostTargetAction_ = ToolBar_->addWidget (PostTargetBox_);
			else
				PostTargetAction_->setVisible (true);
			IProfile *profile = qobject_cast<IProfile*> (Id2Account_ [id]->GetProfile ());
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
			ibsw->SetAccount (Id2Account_ [id]->GetObject ());
			Ui_.Tools_->addItem (w, ibsw->GetName ());
		}

		LoadDrafts ();
	}

	void BlogiqueWidget::saveEntry ()
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		const Event& e = GetCurrentEvent ();
		if (!e.IsEmpty ())
			try
			{
				if (DraftID_ == -1 )
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

		const Event& e = GetCurrentEvent ();
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

	void BlogiqueWidget::submit (const Event& event)
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		const auto& e = event.IsEmpty () ? GetCurrentEvent () : event;

		if (!e.IsEmpty ())
			acc->submit (e);
	}

	void BlogiqueWidget::saveSplitterPosition (int, int)
	{
		XmlSettingsManager::Instance ().setProperty ("MainSplitterPosition",
				Ui_.MainSplitter_->saveState ());
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

// 	void BlogiqueWidget::on_UpdateEntrysList__released ()
// 	{
// 		if (!AccountsBox_->currentIndex ())
// 		{
// 			//TODO mesage box with error
// 			return;
// 		}
//
// 		QInputDialog dlg;
// 		dlg.setInputMode (QInputDialog::IntInput);
// 		dlg.setLabelText (tr ("Number of entries to fetch:"));
// 		dlg.setOkButtonText (tr ("Fetch"));
// 		dlg.setIntStep (1);
// 		dlg.setIntValue (20);
// 		dlg.setIntRange (0, 50);
// 		if (dlg.exec () == QDialog::Rejected)
// 			return;
//
// 		int count = dlg.intValue ();
// 	}

	void BlogiqueWidget::on_RemoveDraft__released ()
	{
		if (!AccountsBox_->currentIndex ())
			return;

		if (XmlSettingsManager::Instance ()
				.Property ("ConfirmDraftRemoving", true).toBool ())
		{
			QMessageBox mbox (QMessageBox::Question,
					"LeechCraft",
					tr ("Do you really want to delete selected draft?"),
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

		QModelIndex idx = Ui_.LocalEntriesView_->currentIndex ();
		idx = idx.sibling (idx.row (), DraftColumns::Date);
		RemoveDraft (idx.data (EntryIdRole::DBIdRole).toLongLong ());
		DraftsViewModel_->removeRow (idx.row ());
	}

	void BlogiqueWidget::on_PublishDraft__released ()
	{
		if (!AccountsBox_->currentIndex ())
			return;

		QModelIndex idx = Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		idx = idx.sibling (idx.row (), DraftColumns::Date);
		const Event& e = LoadFullDraft (acc->GetAccountID (),
				idx.data (EntryIdRole::DBIdRole).toLongLong ());

		submit (e);
		//TODO remove after publish
	}

	void BlogiqueWidget::on_LocalEntriesView__doubleClicked (const QModelIndex& index)
	{
		XmlSettingsManager::Instance ()
				.property ("OpenDraftByDblClick").toString () == "CurrentTab" ?
			handleOpenInCurrentTab (index) :
			handleOpenInNewTab (index);
	}

	void BlogiqueWidget::handleOpenInCurrentTab (const QModelIndex& index)
	{
		QModelIndex idx = index.isValid () ?
			index :
			Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		idx = idx.sibling (idx.row (), DraftColumns::Date);
		const Event& e = LoadFullDraft (acc->GetAccountID (),
				idx.data (EntryIdRole::DBIdRole).toLongLong ());

		FillWidget (e);
	}

	void BlogiqueWidget::handleOpenInNewTab (const QModelIndex& index)
	{
		QModelIndex idx = index.isValid () ?
				index :
				Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		idx = idx.sibling (idx.row (), DraftColumns::Date);
		const Event& e = LoadFullDraft (acc->GetAccountID (),
				idx.data (EntryIdRole::DBIdRole).toLongLong ());

		auto newTab = new BlogiqueWidget;
		connect (newTab,
				SIGNAL (removeTab (QWidget*)),
				&Core::Instance (),
				SIGNAL (removeTab (QWidget*)));

		newTab->FillWidget (e, acc->GetAccountID ());
		emit addNewTab ("Blogique", newTab);
	}
}
}

