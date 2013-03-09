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

#include "profilewidget.h"
#include <QtDebug>
#include <QMessageBox>
#include <util/util.h>
#include "ljprofile.h"
#include "ljaccount.h"
#include "ljfriendentry.h"
#include "frienditemdelegate.h"
#include "xmlsettingsmanager.h"
#include "addeditentrydialog.h"
#include "friendsmodel.h"
#include "core.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	ProfileWidget::ProfileWidget (LJProfile *profile, QWidget *parent)
	: QWidget (parent)
	, Profile_ (profile)
	, FriendsModel_ (new FriendsModel (this))
	, CommunitiesModel_ (new QStandardItemModel (this))
<<<<<<< HEAD
	, MessagesModel_ (new QStandardItemModel (this))
	, PageNumber_ (0)
	, MessageCountOnPage_ (20)
=======
>>>>>>> parent of 2c57894... Blogique Metida: workng with messages
	{
		Ui_.setupUi (this);

		

		FillMessagesUi ();

		Ui_.FriendsView_->setModel (FriendsModel_);
		Ui_.FriendsView_->setDropIndicatorShown (true);

		connect (FriendsModel_,
				SIGNAL (userGroupChanged (const QString&, const QString&,
						const QString&, int)),
				this,
				SLOT (handleUserGroupChanged (const QString&, const QString&,
						const QString&, int)));

		FriendItemDelegate *friendDelegate = new FriendItemDelegate (Ui_.FriendsView_);
		connect (this,
				SIGNAL (coloringItemChanged ()),
				friendDelegate,
				SLOT (handleColoringItemChanged ()));
		Ui_.FriendsView_->setItemDelegate (friendDelegate);

		Ui_.CommunitiesView_->setModel (CommunitiesModel_);

		Ui_.ColoringFriendsList_->setChecked (XmlSettingsManager::Instance ()
				.Property ("ColoringFriendsList", true).toBool ());

		updateProfile ();

		if (XmlSettingsManager::Instance ().Property ("FirstInboxRequest", true).toBool ())
			Profile_->RequestInbox ();
	}

	void ProfileWidget::RereadProfileData ()
	{
		//TODO
		const LJProfileData& data = Profile_->GetProfileData ();

		Ui_.JournalName_->setText (data.FullName_);
		IAccount *acc = qobject_cast<IAccount*> (Profile_->GetParentAccount ());
		const QString& path = Util::CreateIfNotExists ("blogique/metida/avatars")
				.absoluteFilePath (acc->GetAccountID ().toBase64 ().replace ('/', '_'));
		Ui_.JournalPic_->setPixmap (QPixmap (path));

		ReFillModels ();
	}

	void ProfileWidget::FillFriends (const QList<LJFriendGroup>& groups)
	{
		for (const auto& group : groups)
		{
			QStandardItem *item = new QStandardItem (group.Name_);
			item->setData (group.RealId_, ItemGroupRoles::GroupId);
			Item2FriendGroup_ [item] = group;
			item->setEditable (false);
			FriendsModel_->appendRow (item);
		}

		QStandardItem *withoutGroupItem = 0;
		for (const auto& fr : Profile_->GetFriends ())
		{
			QStandardItem *item = new QStandardItem (fr->GetUserName ());
			QStandardItem *itemName = new QStandardItem (fr->GetFullName ());

			QIcon icon;
			if (fr->GetFriendOf () &&
					fr->GetMyFriend ())
				icon = Core::Instance ().GetCoreProxy ()->GetIcon ("im-msn");
			else if (fr->GetFriendOf ())
				icon = Core::Instance ().GetCoreProxy ()->GetIcon ("im-user-offline");
			else if (fr->GetMyFriend ())
				icon = Core::Instance ().GetCoreProxy ()->GetIcon ("im-user");
			QStandardItem *itemStatus = new QStandardItem (icon, QString ());
			QStandardItem *itemBirthday = new QStandardItem (fr->GetBirthday ());
			Item2Friend_ [item] = fr;

			item->setData (fr->GetBGColor ().name (), ItemColorRoles::BackgroundColor);
			item->setData (fr->GetFGColor ().name (), ItemColorRoles::ForegroundColor);

			bool found = false;
			for (const auto& parentItem : Item2FriendGroup_.keys ())
			{
				if (Item2FriendGroup_ [parentItem].RealId_ == fr->GetGroupMask ())
				{
					parentItem->appendRow ({ item, itemStatus, itemName, itemBirthday });
					found = true;
				}
			}

			if (!found)
			{
				if (!withoutGroupItem)
				{
					withoutGroupItem = new QStandardItem (tr ("Without group"));
					withoutGroupItem->setData (-1, ItemGroupRoles::GroupId);
					FriendsModel_->appendRow (withoutGroupItem);
				}

				withoutGroupItem->appendRow ({ item, itemStatus, itemName, itemBirthday });
			}
		}

		Ui_.FriendsView_->header ()->setResizeMode (QHeaderView::ResizeToContents);
	}

	void ProfileWidget::FillCommunities (const QStringList& communities)
	{
		const QIcon& icon = Core::Instance ().GetCoreProxy ()->GetIcon ("system-users");
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

		FriendsModel_->clear ();
		FriendsModel_->setHorizontalHeaderLabels ({ tr ("UserName"),
				tr ("Status"), tr ("FullName"), tr ("Birthday") });
		Item2Friend_.clear ();
		Item2FriendGroup_.clear ();
		FillFriends (data.FriendGroups_);

		CommunitiesModel_->clear ();
		CommunitiesModel_->setHorizontalHeaderLabels ({ tr ("Name") });
		FillCommunities (data.Communities_);
	}

	void ProfileWidget::FillMessagesUi ()
	{
		Ui_.Category_->addItems ({ tr ("All"),
				tr ("Incoming"),
				tr ("Friend updates"),
				tr ("Entries and comments"),
				tr ("Flagged"),
				tr ("Sent") });
		Ui_.FriendsUpdates_->addItems ({ tr ("All"),
				tr ("Birthdays"),
				tr ("New friends") });

		Ui_.Category_->setCurrentIndex (MCAll);
		
		Ui_.Next_->setIcon (Core::Instance ().GetCoreProxy ()->GetIcon ("go-next"));
<<<<<<< HEAD
		Ui_.Previous_->setIcon (Core::Instance ().GetCoreProxy ()->GetIcon ("go-previous"));

		Ui_.InboxView_->setModel (MessagesModel_);

		MessagesModel_->setHorizontalHeaderLabels ({ tr ("Date"),
				tr ("Read"),
				tr ("Subject"),
				tr ("Description") });

		FillInboxView (20, 0);
	}

	namespace
	{
		LJInbox::MessageType MessageTypeFromCategory (ProfileWidget::MessageCategory category,
				ProfileWidget::FriendUpdatesCategory friendsCategory)
		{
			switch (category)
			{
			case ProfileWidget::MCAll:
				return LJInbox::MessageType::NoType;
			case ProfileWidget::MCIncoming:
				return LJInbox::MessageType::UserMessageRecvd;
			case ProfileWidget::MCFriendUpdates:
				switch (friendsCategory)
				{
				case ProfileWidget::FUCBirthdays:
					return LJInbox::MessageType::Birthday;
				case ProfileWidget::FUCNewFriends:
					return LJInbox::MessageType::Friended;
				case ProfileWidget::FUCAll:
				default:
					return LJInbox::MessageType::NoType;
				}
			case ProfileWidget::MCEntriesAndComments:
				return LJInbox::MessageType::JournalNewComment;
			case ProfileWidget::MCSent:
				return LJInbox::MessageType::UserMessageSent;
			default:
				return LJInbox::MessageType::NoType;
			}
		}

		QPair<QString, QString> GetTextFromMessageType (LJInbox::MessageType mt)
		{
			switch (mt)
			{
			case LJInbox::MessageType::NoType:
				return { QString (), QString () };
			case LJInbox::MessageType::Friended:
				return { QObject::tr ("Friends event"),
						QObject::tr ("One user has friended you") };
			case LJInbox::MessageType::Defriended:
				return { QObject::tr ("Friends event"),
						QObject::tr ("One user has defriended you") };
			case LJInbox::MessageType::InvitedFriendJoins:
				return { QObject::tr ("Friends event"),
						QObject::tr ("Invited friend joins") };
			case LJInbox::MessageType::Birthday:
				return { QObject::tr ("Birthday event"),
						QObject::tr ("One of your friends has birthday") };
			case LJInbox::MessageType::CommunityInvite:
				return { QObject::tr ("Commumity event"),
						QObject::tr ("Invite to community") };
			case LJInbox::MessageType::CommunityJoinApprove:
				return { QObject::tr ("Commumity event"),
						QObject::tr ("Your join request was approved") };
			case LJInbox::MessageType::CommunityJoinReject:
				return { QObject::tr ("Commumity event"),
						QObject::tr ("Your join request was rejected") };
			case LJInbox::MessageType::CommunityJoinRequest:
				return { QObject::tr ("Commumity event"),
						QObject::tr ("You have got join request") };
			case LJInbox::MessageType::JournalNewComment:
				return { QObject::tr ("Journal event"),
						QObject::tr ("You have new comment") };
			case LJInbox::MessageType::JournalNewEntry:
				return { QObject::tr ("Journal event"),
						QObject::tr ("You have new entry") };
			case LJInbox::MessageType::NewUserpic:
				return { QObject::tr ("User event"),
						QObject::tr ("Userpic was updated") };
			case LJInbox::MessageType::UserMessageRecvd:
				return { QObject::tr ("User event"),
						QObject::tr ("Message received") };
			case LJInbox::MessageType::UserMessageSent:
				return { QObject::tr ("User event"),
						QObject::tr ("Message sent") };
			case LJInbox::MessageType::UserNewComment:
				return { QObject::tr ("User event"),
						QObject::tr ("You have new comment") };
			case LJInbox::MessageType::UserNewEntry:
				return { QObject::tr ("User event"),
						QObject::tr ("You have new entry") };
			case LJInbox::MessageType::UserExpunged:
				return { QObject::tr ("User event"),
						QObject::tr ("User was expunged") };
			case LJInbox::MessageType::NewVGift:
				return { QObject::tr ("User event"),
						QObject::tr ("You got new gift") };
			case LJInbox::MessageType::OfficialPost:
				return { QObject::tr ("Official event"),
						QObject::tr ("New official post in news community") };
			case LJInbox::MessageType::SupOfficialPost:
				return { QObject::tr ("Official"),
						QObject::tr ("New SUP official post") };
			case LJInbox::MessageType::PermSale:
				return { QObject::tr ("Account event"),
						QString () };
			default:
				return { QString (), QString () };
			}
		}

		QList<QStandardItem*> GetRowFromMessage (LJInbox::Message *msg)
		{
			QStandardItem *whenItem = new QStandardItem (msg->When_.toString (Qt::DefaultLocaleShortDate));
			QStandardItem *stateItem = new QStandardItem (msg->State_ == LJInbox::MessageState::Read ? "R" : "U");

			QStandardItem *subjectItem = 0;
			QStandardItem *textItem = 0;
			if (!msg->ExtendedSubject_.isEmpty ())
				subjectItem = new QStandardItem (msg->ExtendedSubject_);
			else if (!msg->TypeString_.isEmpty ())
			{
				subjectItem = new QStandardItem (msg->TypeString_);
				if (msg->TypeString_ == "CommentReply")
					textItem = new QStandardItem (QObject::tr ("You have new comment"));
			}
			else
			{
				auto pair = GetTextFromMessageType (static_cast<LJInbox::MessageType> (msg->Type_));
				subjectItem = new QStandardItem (pair.first);
				textItem = new QStandardItem (pair.second);
			}

			if (!textItem)
				textItem = new QStandardItem (msg->ExtendedText_);

			whenItem->setEditable (false);
			stateItem->setEditable (false);
			subjectItem->setEditable (false);
			textItem->setEditable (false);

			return { whenItem, stateItem, subjectItem, textItem };
		}
	}

	void ProfileWidget::FillInboxView (int limit, int offset)
	{
		MessagesModel_->removeRows (0, MessagesModel_->rowCount ());
		try
		{
			auto mc = static_cast<ProfileWidget::MessageCategory> (Ui_.Category_->
					itemData (Ui_.Category_->currentIndex ()).toInt ());
			auto fuc = static_cast<ProfileWidget::FriendUpdatesCategory> ((mc == MCFriendUpdates) ?
				Ui_.FriendsUpdates_->itemData (Ui_.FriendsUpdates_->currentIndex ()).toInt () :
				FUCAll);
			auto msgs = Core::Instance ().GetLocalStorage ()->GetLimitedMessages (20,
					offset,
					MessageTypeFromCategory (mc, fuc),
					qobject_cast<IAccount*> (Profile_->GetParentAccount ())->
							GetAccountID ());

			for (auto msg : msgs)
			{
				MessagesModel_->appendRow (GetRowFromMessage (msg));
				Ui_.InboxView_->resizeColumnToContents (0);
				Ui_.InboxView_->resizeColumnToContents (1);
				Ui_.InboxView_->resizeColumnToContents (2);
				Ui_.InboxView_->resizeColumnToContents (3);
			}
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
=======
		Ui_.Prevous_->setIcon (Core::Instance ().GetCoreProxy ()->GetIcon ("go-previous"));
>>>>>>> parent of 2c57894... Blogique Metida: workng with messages
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
		emit coloringItemChanged ();
	}

	void ProfileWidget::on_Add__released ()
	{
		std::unique_ptr<AddEditEntryDialog> aeed (new AddEditEntryDialog (Profile_));

		if (aeed->exec () == QDialog::Rejected)
			return;

		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		switch (aeed->GetAddTypeEntry ())
		{
			case ATEFriend:
			{
				const QString& userName = aeed->GetUserName ();
				const QString& bgcolor = aeed->GetBackgroundColorName ();
				const QString& fgcolor = aeed->GetForegroundColorName ();
				uint realId =  aeed->GetGroupRealId ();

				account->AddNewFriend (userName, bgcolor, fgcolor, realId);
				break;
			}
			case ATEGroup:
			{
				int id = Profile_->GetFreeGroupId ();
				if (id == -1)
				{
					QMessageBox::warning (this,
							tr ("Add new group"),
							tr ("You cannot add more groups. The limit of 30 groups is reached."));
					return;
				}

				account->AddGroup (aeed->GetGroupName (), aeed->GetAcccess (), id);
				break;
			}
		}
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

		AddEditEntryDialog dlg (Profile_);
		dlg.ShowAddTypePossibility (false);

		if (!index.parent ().isValid ())
		{
			dlg.SetCurrentAddTypeEntry (ATEGroup);
			dlg.SetGroupName (index.data ().toString ());
			dlg.SetAccess (Item2FriendGroup_ [FriendsModel_->itemFromIndex (index)].Public_);

			if (dlg.exec () == QDialog::Rejected)
				return;

			account->AddGroup (dlg.GetGroupName (), dlg.GetAcccess (),
					Item2FriendGroup_ [FriendsModel_->itemFromIndex (index)].Id_);
		}
		else
		{
			dlg.SetCurrentAddTypeEntry (ATEFriend);
			LJFriendEntry_ptr entry = Item2Friend_ [FriendsModel_->itemFromIndex (index)];
			dlg.SetUserName (entry->GetUserName ());
			dlg.SetBackgroundColor (entry->GetBGColor ());
			dlg.SetForegroundColor (entry->GetFGColor ());
			dlg.SetGroup (entry->GetGroupMask ());

			if (dlg.exec () == QDialog::Rejected)
				return;

			const QString& userName = dlg.GetUserName ();
			const QString& bgcolor = dlg.GetBackgroundColorName ();
			const QString& fgcolor = dlg.GetForegroundColorName ();
			uint realId = dlg.GetGroupRealId ();

			account->AddNewFriend (userName, bgcolor, fgcolor, realId);
		}
	}

	void ProfileWidget::on_Delete__released ()
	{
		auto index = Ui_.FriendsView_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		QString msg;
		index.parent ().isValid () ?
			msg = tr ("Are you sure to delete user <b>%1</b> from your friends.")
					.arg (index.data ().toString ()) :
			msg = tr ("Are you sure to delete group <b>%1</b>"
					"<br><i>Note: all friends in this group will <b>not</b> be deleted.</i>")
					.arg (index.data ().toString ());
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

			if (!index.parent ().isValid ())
				account->DeleteGroup (Item2FriendGroup_ [FriendsModel_->itemFromIndex (index)].Id_);
			else
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

	void ProfileWidget::handleUserGroupChanged (const QString& username,
			const QString& bgColor, const QString& fgColor, int groupId)
	{
		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		account->AddNewFriend (username, bgColor, fgColor, groupId);
	}

	void ProfileWidget::on_Category__currentIndexChanged (int index)
	{
		Ui_.FriendsUpdates_->setVisible (index == MCFriendUpdates);
	}

	void ProfileWidget::on_FriendsUpdates__currentIndexChanged (int index)
	{

	}

<<<<<<< HEAD
	void ProfileWidget::on_Next__released ()
	{
		FillInboxView (20, 20 * ++PageNumber_);
		if (!MessagesModel_->rowCount ())
			on_Previous__released ();
	}

	void ProfileWidget::on_Previous__released ()
	{
		if (PageNumber_ == 0)
			return;

		FillInboxView (20, 20 * --PageNumber_);
	}

=======
>>>>>>> parent of 2c57894... Blogique Metida: workng with messages
}
}
}


