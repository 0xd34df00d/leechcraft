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
#include "draftentrieswidget.h"
#include "localstorage.h"
#include "blogentrieswidget.h"
#include "updateentriesdialog.h"
#include "xmlsettingsmanager.h"
#include "storagemanager.h"

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
	, DraftEntriesWidget_ (new DraftEntriesWidget)
	, BlogEntriesWidget_ (new BlogEntriesWidget)
	, PrevAccountId_ (-1)
	, EntryType_ (EntryType::None)
	, EntryId_ (-1)
	, EntryChanged_ (false)
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

		connect (BlogEntriesWidget_,
				SIGNAL(fillCurrentWidgetWithBlogEntry (Entry)),
				this,
				SLOT (fillCurrentTabWithEntry (Entry)));
		connect (BlogEntriesWidget_,
				SIGNAL(fillNewWidgetWithBlogEntry (Entry,QByteArray)),
				this,
				SLOT (fillNewTabWithEntry (Entry, QByteArray)));
		connect (DraftEntriesWidget_,
				SIGNAL(fillCurrentWidgetWithDraftEntry (Entry)),
				this,
				SLOT (fillCurrentTabWithEntry (Entry)));
		connect (DraftEntriesWidget_,
				SIGNAL(fillNewWidgetWithDraftEntry (Entry, QByteArray)),
				this,
				SLOT (fillNewTabWithEntry (Entry, QByteArray)));

		DraftEntriesWidget_->loadDraftEntries ();
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

		EntryType_ = e.EntryType_;
		EntryId_ = e.EntryId_;
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

		EntryChanged_ = false;
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

			connect (w,
					SIGNAL (textChanged ()),
					this,
					SLOT (handleTextChanged ()));
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
			Ui_.Tools_->removeItem (i);
			w->deleteLater ();
		}
		Ui_.Tools_->addItem (DraftEntriesWidget_, DraftEntriesWidget_->GetName ());
	}

	void BlogiqueWidget::RemovePostingTargetsWidget ()
	{
		if (PostTargetAction_ &&
				PostTargetAction_->isVisible ())
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
		EntryChanged_ = false;
	}

	Entry BlogiqueWidget::GetCurrentEntry ()
	{
		const QString& content = PostEdit_->GetContents (ContentType::HTML);
		if (content.isEmpty ())
		{
			QMessageBox::warning (this,
					tr ("LeechCraft"),
					tr ("Entry can't be emprty."));
			return Entry ();
		}

		Entry e;
		for (auto w : SidePluginsWidgets_)
		{
			auto ibsw = qobject_cast<IBlogiqueSideWidget*> (w);
			if (!ibsw)
				continue;

			switch (ibsw->GetWidgetType ())
			{
			case SideWidgetType::PostOptionsSideWidget:
			{
				e.PostOptions_.unite (ibsw->GetPostOptions ());
				auto ipow = qobject_cast<IPostOptionsWidget*> (w);
				if (!ipow)
					continue;

				e.Date_ = ipow->GetPostDate ();
				e.Tags_ = ipow->GetTags ();
				break;
			}
			case SideWidgetType::CustomSideWidget:
				e.CustomData_.unite (ibsw->GetCustomData ());
				break;
			default:
				break;
			}
		}

		e.Target_ = PostTargetBox_->currentText ();
		e.Content_ = content;
		e.Subject_ = Ui_.Subject_->text ();
		e.EntryType_ = EntryType_;
		e.EntryId_ = EntryId_;
		e.Date_ = !e.Date_.isNull () ? e.Date_ : QDateTime::currentDateTime ();

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
		BlogEntriesWidget_->SetAccount (account);

		auto ibp = qobject_cast<IBloggingPlatform*> (account->
				GetParentBloggingPlatform ());

		BlogEntriesWidget_->clear ();

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

		bool exists = false;
		for (int i = 0; i < Ui_.Tools_->count (); ++i)
			if (Ui_.Tools_->widget (i) == BlogEntriesWidget_)
			{
				exists = true;
				break;
			}
		if (!exists)
			Ui_.Tools_->insertItem (0, BlogEntriesWidget_, BlogEntriesWidget_->GetName ());

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

		PrevAccountId_ = id;
	}

	void BlogiqueWidget::fillCurrentTabWithEntry (const Entry& entry)
	{
		if (EntryChanged_)
		{
			IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
			if (!acc)
				return;
			int res = QMessageBox::question (this,
					"LeechCraft Blogique",
					tr ("You have unsaved changes in your current tab."
						" Do you want to open this entry in a new tab instead?"),
					QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			switch (res)
			{
			case QMessageBox::Yes:
				fillNewTabWithEntry (entry, acc->GetAccountID ());
				break;
			case QMessageBox::No:
				FillWidget (entry);
				break;
			case QMessageBox::Cancel:
			default:
				return;
			}
		}
		else
			FillWidget (entry);
	}

	void BlogiqueWidget::fillNewTabWithEntry (const Entry& entry,
			const QByteArray& accountId)
	{
		auto w = Core::Instance ().CreateBlogiqueWidget ();
		w->FillWidget (entry, accountId);
		emit addNewTab ("Blogique", w);
	}

	void BlogiqueWidget::handleTextChanged ()
	{
		EntryChanged_ = true;
	}

	void BlogiqueWidget::newEntry ()
	{
		if (EntryChanged_)
		{
			int res = QMessageBox::question (this,
					"LeechCraft Blogique",
					tr ("Do you want to save current entry?"),
					QMessageBox::Yes | QMessageBox::No);
			switch (res)
			{
			case QMessageBox::Yes:
				saveEntry ();
				ClearEntry ();
				break;
			case QMessageBox::No:
				ClearEntry ();
				break;
			default:
				return;
			}
		}
		else
			ClearEntry ();
	}

	void BlogiqueWidget::saveEntry (const Entry& entry)
	{
		EntryChanged_ = false;
		EntryType_ = EntryType::Draft;
		const Entry& e = entry.IsEmpty () ?
			GetCurrentEntry () :
			entry;
		if (!e.IsEmpty ())
		{
			try
			{
				switch (e.EntryType_)
				{
				case EntryType::Draft:
					EntryId_ = Core::Instance ().GetStorageManager ()->UpdateDraft (e, EntryId_);
					break;
				case EntryType::BlogEntry:
				case EntryType::None:
					EntryId_ = Core::Instance ().GetStorageManager ()->SaveNewDraft (e);
					break;
				}
			}
			catch (const std::runtime_error& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error saving draft"
						<< e.what ();
			}
		}
		else
			EntryType_ = EntryType::None;

		DraftEntriesWidget_->loadDraftEntries ();
	}

	void BlogiqueWidget::saveNewEntry (const Entry& entry)
	{
		EntryChanged_ = false;
		EntryType_ = EntryType::Draft;
		const Entry& e = entry.IsEmpty () ?
			GetCurrentEntry () :
			entry;

		if (!e.IsEmpty ())
		{
			try
			{
				EntryId_ = Core::Instance ().GetStorageManager ()->SaveNewDraft (e);
			}
			catch (const std::runtime_error& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error saving draft"
						<< e.what ();
			}
		}
		else
			EntryType_ = EntryType::None;

		DraftEntriesWidget_->loadDraftEntries ();
	}

	void BlogiqueWidget::submit (const Entry& event)
	{
		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		EntryChanged_ = false;
		const auto& e = event.IsEmpty () ?
			GetCurrentEntry () :
			event;

		if (!e.IsEmpty ())
		{
			if (EntryType_ == EntryType::BlogEntry)
			{
				QMessageBox mbox (QMessageBox::Question,
						"LeechCraft",
						tr ("Do you want to update entry or to post new one?"),
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

}
}

