/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "profilewidget.h"
#include <stdexcept>
#include <QtDebug>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDir>
#include <QAction>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sys/paths.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "ljprofile.h"
#include "ljaccount.h"
#include "ljfriendentry.h"
#include "frienditemdelegate.h"
#include "xmlsettingsmanager.h"
#include "addeditentrydialog.h"
#include "friendsproxymodel.h"
#include "sendmessagedialog.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	ProfileWidget::ProfileWidget (LJProfile *profile, const ICoreProxy_ptr& proxy, QWidget *parent)
	: QWidget (parent)
	, Profile_ (profile)
	, Proxy_ (proxy)
	, FriendsModel_ (new QStandardItemModel (this))
	, FriendsProxyModel_ (new FriendsProxyModel (this))
	, GroupsModel_ (new QStandardItemModel (this))
	, FriendsInGroupModel_ (new QStandardItemModel (this))
	, FriendsNotInGroupModel_ (new QStandardItemModel (this))
	, CommunitiesModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		FriendsProxyModel_->setSourceModel (FriendsModel_);
		Ui_.FriendsView_->setModel (FriendsProxyModel_);
		Ui_.FriendsView_->setDropIndicatorShown (true);
		FriendsModel_->setHorizontalHeaderLabels ({ tr ("Username"),
				tr ("Status"), tr ("Full name"), tr ("Birthday") });

		const auto friendDelegate = new FriendItemDelegate (Ui_.FriendsView_);
		Ui_.FriendsView_->setItemDelegate (friendDelegate);
		QAction *newFriend = new QAction (tr ("Add friend"), this);
		newFriend->setProperty ("ActionIcon", "list-add");
		connect (newFriend,
				SIGNAL (triggered ()),
				this,
				SLOT (on_Add__released ()));
		QAction *deleteFriend = new QAction (tr ("Delete friend"), this);
		deleteFriend->setProperty ("ActionIcon", "list-remove");
		connect (deleteFriend,
				SIGNAL (triggered ()),
				this,
				SLOT (on_Delete__released ()));
		QAction *editFriend = new QAction (tr ("Edit friend"), this);
		editFriend->setProperty ("ActionIcon", "edit-select");
		connect (editFriend,
				SIGNAL (triggered ()),
				this,
				SLOT (on_Edit__released ()));
		QAction *readJournal = new QAction (tr ("Read journal"), this);
		readJournal->setProperty ("ActionIcon", "text-field");
		connect (readJournal,
				SIGNAL (triggered ()),
				this,
				SLOT (handleReadJournal ()));
		QAction *sendMessage = new QAction (tr ("Send message"), this);
		sendMessage->setProperty ("ActionIcon", "mail-mark-unread");
		connect (sendMessage,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSendMessage ()));
		connect (Ui_.FriendsView_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handleFriendsViewDoubleClicked (QModelIndex)));
		Ui_.FriendsView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		Ui_.FriendsView_->addActions ({ readJournal,
				Util::CreateSeparator (Ui_.FriendsView_),
				sendMessage,
				Util::CreateSeparator (Ui_.FriendsView_),
				newFriend, editFriend, deleteFriend });

		Ui_.Groups_->setModel (GroupsModel_);
		Ui_.Groups_->setHeaderHidden (true);
		Ui_.InGroupUsers_->setModel (FriendsInGroupModel_);
		Ui_.InGroupUsers_->setHeaderHidden (true);
		Ui_.NotInGroupUsers_->setModel (FriendsNotInGroupModel_);
		Ui_.NotInGroupUsers_->setHeaderHidden (true);

		Ui_.CommunitiesView_->setModel (CommunitiesModel_);
		Ui_.CommunitiesView_->setHeaderHidden (true);
		Ui_.CommunitiesView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		connect (Ui_.CommunitiesView_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handleCommunitiesViewDoubleClicked (QModelIndex)));
		QAction *readCommunity = new QAction (tr ("Read community"), this);
		readCommunity->setProperty ("ActionIcon", "text-field");
		Ui_.CommunitiesView_->addAction (readCommunity);
		connect (readCommunity,
				SIGNAL (triggered ()),
				this,
				SLOT (handleReadCommunity ()));

		Ui_.ColoringFriendsList_->setChecked (XmlSettingsManager::Instance ()
				.Property ("ColoringFriendsList", true).toBool ());

		updateProfile ();

		connect (Ui_.Filter_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (handleFriendFilterTextChanged (QString)));

		Ui_.Groups_->setContextMenuPolicy (Qt::ActionsContextMenu);
		QAction *newGroup = new QAction (tr ("Add group"), this);
		newGroup->setProperty ("ActionIcon", "list-add");
		connect (newGroup,
				SIGNAL (triggered ()),
				this,
				SLOT (addNewGroup ()));
		QAction *deleteGroup = new QAction (tr ("Delete group"), this);
		deleteGroup->setProperty ("ActionIcon", "list-remove");
		connect (deleteGroup,
				SIGNAL (triggered ()),
				this,
				SLOT (deleteGroup ()));
		QAction *editGroup = new QAction (tr ("Edit group"), this);
		editGroup->setProperty ("ActionIcon", "edit-select");
		connect (editGroup,
				SIGNAL (triggered ()),
				this,
				SLOT (editGroup ()));

		Ui_.Groups_->addActions ({ newGroup, editGroup, deleteGroup });
	}

	void ProfileWidget::RereadProfileData ()
	{
		//TODO
		const LJProfileData& data = Profile_->GetProfileData ();

		Ui_.JournalName_->setText (data.FullName_);
		IAccount *acc = qobject_cast<IAccount*> (Profile_->GetParentAccount ());
		const QString& path = Util::GetUserDir (Util::UserDir::Cache, "blogique/metida/avatars")
				.absoluteFilePath (acc->GetAccountID ().toBase64 ().replace ('/', '_'));
		Ui_.JournalPic_->setPixmap (QPixmap (path));
		ReFillModels ();
	}

	void ProfileWidget::FillFriends (const QList<LJFriendEntry_ptr>& friends)
	{
		for (const auto& fr : friends)
		{
			Username2Friend_ [fr->GetUserName ()] = fr;

			QStandardItem *item = new QStandardItem (fr->GetUserName ());
			QStandardItem *itemName = new QStandardItem (fr->GetFullName ());

			const auto iconMgr = Proxy_->GetIconThemeManager ();

			QIcon icon;
			FriendsProxyModel::FriendStatus status = FriendsProxyModel::FSFriendOf;
			if (fr->GetFriendOf () &&
					fr->GetMyFriend ())
			{
				icon = iconMgr->GetIcon ("im-msn");
				status = FriendsProxyModel::FSBothFriends;
			}
			else if (fr->GetFriendOf ())
			{
				icon = iconMgr->GetIcon ("im-user-offline");
				status = FriendsProxyModel::FSFriendOf;
			}
			else if (fr->GetMyFriend ())
			{
				icon = iconMgr->GetIcon ("im-user");
				status = FriendsProxyModel::FSMyFriend;
			}
			QStandardItem *itemStatus = new QStandardItem (icon, QString ());
			itemStatus->setData (status, FriendsProxyModel::FRFriendStatus);
			QStandardItem *itemBirthday = new QStandardItem (fr->GetBirthday ());
			Item2Friend_.remove (Friend2Item_.value (fr));
			Friend2Item_ [fr] = item;
			Item2Friend_ [item] = fr;

			Friend2Item_ [fr] = item;

			item->setData (fr->GetBGColor ().name (), ItemColorRoles::BackgroundColor);
			item->setData (fr->GetFGColor ().name (), ItemColorRoles::ForegroundColor);

			FriendsModel_->appendRow ({ item, itemStatus, itemName, itemBirthday });
		}

		Ui_.FriendsView_->header ()->setSectionResizeMode (QHeaderView::ResizeToContents);
	}

	void ProfileWidget::FillGroups (const QList<LJFriendGroup>& groups)
	{
		for (const auto& group : groups)
		{
			QStandardItem *item = new QStandardItem (group.Name_);
			item->setData (group.RealId_, ItemGroupRoles::GroupId);
			Item2FriendGroup_ [item] = group;
			item->setEditable (false);
			GroupsModel_->appendRow (item);
		}
	}

	void ProfileWidget::FillCommunities (const QStringList& communities)
	{
		const QIcon& icon = Proxy_->GetIconThemeManager ()->GetIcon ("system-users");
		for (const auto& community : communities)
		{
			QStandardItem *item = new QStandardItem (icon, community);
			item->setEditable (false);
			CommunitiesModel_->appendRow (item);
		}
	}

	void ProfileWidget::ReFillModels ()
	{
		const LJProfileData& data = Profile_->GetProfileData ();
		FriendsModel_->removeRows (0, FriendsModel_->rowCount ());
		Item2Friend_.clear ();
		Friend2Item_.clear ();
		FillFriends (data.Friends_);

		GroupsModel_->removeRows (0, GroupsModel_->rowCount ());
		Item2FriendGroup_.clear ();
		FillGroups (data.FriendGroups_);

		CommunitiesModel_->removeRows (0, CommunitiesModel_->rowCount());
		FillCommunities (data.Communities_);
	}

	void ProfileWidget::updateProfile ()
	{
		if (Profile_)
			RereadProfileData ();
		else
			qWarning () << Q_FUNC_INFO
					<< "Profile is set to 0";
	}

	void ProfileWidget::on_ColoringFriendsList__toggled (bool toggle)
	{
		XmlSettingsManager::Instance ().setProperty ("ColoringFriendsList", toggle);
	}

	void ProfileWidget::on_Add__released ()
	{
		AddEditEntryDialog aeed (Profile_, ATEFriend);
		if (aeed.exec () == QDialog::Rejected)
			return;

		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		const QString& userName = aeed.GetUserName ();
		const QString& bgcolor = aeed.GetBackgroundColorName ();
		const QString& fgcolor = aeed.GetForegroundColorName ();
		const uint groupMask = aeed.GetGroupMask ();

		account->AddNewFriend (userName, bgcolor, fgcolor, groupMask);
	}

	void ProfileWidget::on_Edit__released ()
	{
		auto index = Ui_.FriendsView_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		QString msg;
		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		AddEditEntryDialog dlg (Profile_, ATENone);
		dlg.ShowAddTypePossibility (false);
		dlg.SetCurrentAddTypeEntry (ATEFriend);
		LJFriendEntry_ptr entry = Item2Friend_ [FriendsModel_->itemFromIndex (FriendsProxyModel_->mapToSource (index))];
		dlg.SetUserName (entry->GetUserName ());
		dlg.SetBackgroundColor (entry->GetBGColor ());
		dlg.SetForegroundColor (entry->GetFGColor ());
		dlg.SetGroupMask (entry->GetGroupMask ());

		if (dlg.exec () == QDialog::Rejected)
			return;

		const QString& userName = dlg.GetUserName ();
		const QString& bgcolor = dlg.GetBackgroundColorName ();
		const QString& fgcolor = dlg.GetForegroundColorName ();
		const uint mask = dlg.GetGroupMask ();

		account->AddNewFriend (userName, bgcolor, fgcolor, mask);
	}

	void ProfileWidget::on_Delete__released ()
	{
		auto index = Ui_.FriendsView_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		const QString& msg = tr ("Are you sure you want to delete %1 from your friends?")
				.arg ("<em>" + index.data ().toString () + "</em>");
		int res = QMessageBox::question (this,
				tr ("Change friendslist"),
				msg,
				QMessageBox::Ok | QMessageBox::Cancel,
				QMessageBox::Cancel);

		if (res == QMessageBox::Ok)
		{
			LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
			if (!account)
				return;
			account->DeleteFriend (index.data ().toString ());
		}
	}

	void ProfileWidget::on_UpdateProfile__released ()
	{
		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;
		account->updateProfile ();
	}

	void ProfileWidget::on_Groups__clicked (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		FriendsInGroupModel_->removeRows (0, FriendsInGroupModel_->rowCount ());
		FriendsNotInGroupModel_->removeRows (0, FriendsNotInGroupModel_->rowCount ());

		const auto& group = Item2FriendGroup_ [GroupsModel_->itemFromIndex (index)];
		for (const auto& friendEntry : Item2Friend_.values ())
		{
			QStandardItem * item = new QStandardItem (friendEntry->GetUserName ());
			item->setEditable (false);
			((friendEntry->GetGroupMask () >> group.Id_) & 1) ?
				FriendsInGroupModel_->appendRow (item) :
				FriendsNotInGroupModel_->appendRow (item);
		}
		FriendsInGroupModel_->sort (0, Qt::AscendingOrder);
		FriendsNotInGroupModel_->sort (0, Qt::AscendingOrder);
	}

	void ProfileWidget::on_AddUserToGroup__released ()
	{
		const auto& index = Ui_.NotInGroupUsers_->currentIndex ();
		if (!index.isValid ())
			return;

		const auto& groupIndex = Ui_.Groups_->selectionModel ()->selectedRows ().value (0);
		if (!groupIndex.isValid ())
			return;
		const auto& group = Item2FriendGroup_ [GroupsModel_->itemFromIndex (groupIndex)];
		auto item = new QStandardItem (index.data ().toString ());
		item->setEditable (false);
		FriendsNotInGroupModel_->removeRow (index.row ());
		FriendsInGroupModel_->appendRow (item);
		if (Username2Friend_.contains (item->text ()))
		{
			const auto& friendEntry = Username2Friend_ [item->text ()];
			uint groupMask = friendEntry->GetGroupMask ();
			groupMask = groupMask | (1 << group.Id_);
			handleUserGroupChanged (friendEntry->GetUserName (),
					friendEntry->GetBGColor ().name (), friendEntry->GetFGColor ().name (),
					groupMask);
		}
		FriendsInGroupModel_->sort (0, Qt::AscendingOrder);
		FriendsNotInGroupModel_->sort (0, Qt::AscendingOrder);
	}

	void ProfileWidget::on_RemoveUserFromGroup__released ()
	{
		const auto& index = Ui_.InGroupUsers_->currentIndex ();
		if (!index.isValid ())
			return;

		const auto& groupIndex = Ui_.Groups_->selectionModel ()->selectedRows ().value (0);
		if (!groupIndex.isValid ())
			return;
		const auto& group = Item2FriendGroup_ [GroupsModel_->itemFromIndex (groupIndex)];
		auto item = new QStandardItem (index.data ().toString ());
		item->setEditable (false);
		FriendsInGroupModel_->removeRow (index.row ());
		FriendsNotInGroupModel_->appendRow (item);
		if (Username2Friend_.contains (item->text ()))
		{
			const auto& friendEntry = Username2Friend_ [item->text ()];
			uint groupMask = friendEntry->GetGroupMask ();
			groupMask = groupMask & ~(1 << group.Id_);
			handleUserGroupChanged (friendEntry->GetUserName (),
					friendEntry->GetBGColor ().name (), friendEntry->GetFGColor ().name (),
					groupMask);
		}
		FriendsInGroupModel_->sort (0, Qt::AscendingOrder);
		FriendsNotInGroupModel_->sort (0, Qt::AscendingOrder);
	}

	void ProfileWidget::on_SendMessage__released ()
	{
		SendMessageDialog dlg (Profile_);
		dlg.setWindowModality (Qt::WindowModal);
		if (dlg.exec () == QDialog::Rejected)
			return;

		if (auto acc = qobject_cast<LJAccount*> (Profile_->GetParentAccount ()))
			acc->SendMessage (dlg.GetAddresses (), dlg.GetSubject (), dlg.GetText ());
	}

	void ProfileWidget::handleUserGroupChanged (const QString& username,
			const QString& bgColor, const QString& fgColor, int groupMask)
	{
		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		account->AddNewFriend (username, bgColor, fgColor, groupMask);
	}

	void ProfileWidget::handleFriendFilterTextChanged (const QString& text)
	{
		FriendsProxyModel_->SetFilterString (text);
	}

	void ProfileWidget::handleReadJournal ()
	{
		auto index = Ui_.FriendsView_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeEntity (QUrl (QString ("http://%1.livejournal.com")
					.arg (index.data ().toString ())),
				QString (),
				OnlyHandle | FromUserInitiated));
	}

	void ProfileWidget::handleSendMessage ()
	{
		auto index = Ui_.FriendsView_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		SendMessageDialog dlg (Profile_);
		dlg.setWindowModality (Qt::WindowModal);
		dlg.SetAddresses ({ index.data ().toString () });
		if (dlg.exec () == QDialog::Rejected)
			return;

		if (auto acc = qobject_cast<LJAccount*> (Profile_->GetParentAccount ()))
			acc->SendMessage (dlg.GetAddresses (), dlg.GetSubject (), dlg.GetText ());
	}

	void ProfileWidget::handleReadCommunity ()
	{
		auto index = Ui_.CommunitiesView_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeEntity (QUrl (QString ("http://%1.livejournal.com")
					.arg (index.data ().toString ())),
				QString (),
				OnlyHandle | FromUserInitiated));
	}

	void ProfileWidget::handleFriendsViewDoubleClicked (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		handleReadJournal ();
	}

	void ProfileWidget::handleCommunitiesViewDoubleClicked (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		handleReadCommunity ();
	}

	void ProfileWidget::addNewGroup ()
	{
		AddEditEntryDialog aeed (Profile_, ATEGroup);
		if (aeed.exec () == QDialog::Rejected)
			return;
		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		int id = Profile_->GetFreeGroupId ();
		if (id == -1)
		{
			QMessageBox::critical (this,
					tr ("Adding new group"),
					tr ("You cannot add more groups: the limit of 30 groups is reached."));
			return;
		}

		account->AddGroup (aeed.GetGroupName (), aeed.GetAcccess (), id);
	}

	void ProfileWidget::deleteGroup ()
	{
		auto index = Ui_.Groups_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		const QString& msg = tr ("Are you sure you want to delete the group %1?"
					"<br><i>Note: all friends in this group will <b>not</b> be deleted.</i>")
							.arg ("<em>" + index.data ().toString () + "</em>");
		int res = QMessageBox::question (this,
				tr ("Change groups"),
				msg,
				QMessageBox::Ok | QMessageBox::Cancel,
				QMessageBox::Cancel);

		if (res == QMessageBox::Ok)
		{
			LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
			if (!account)
				return;
			account->DeleteGroup (Item2FriendGroup_ [FriendsModel_->itemFromIndex (index)].Id_);
		}
	}

	void ProfileWidget::editGroup ()
	{
		auto index = Ui_.Groups_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		QString msg;
		const auto& item = Item2FriendGroup_ [GroupsModel_->itemFromIndex (index)];
		AddEditEntryDialog dlg (Profile_, ATENone);
		dlg.ShowAddTypePossibility (false);
		dlg.SetCurrentAddTypeEntry (ATEGroup);
		dlg.SetGroupName (index.data ().toString ());
		dlg.SetAccess (item.Public_);

		if (dlg.exec () == QDialog::Rejected)
			return;

		account->AddGroup (dlg.GetGroupName (), dlg.GetAcccess (), item.Id_);
	}
}
}
}
