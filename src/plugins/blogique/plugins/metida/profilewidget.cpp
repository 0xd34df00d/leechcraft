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
#include <QStandardItemModel>
#include <QMessageBox>
#include <util/util.h>
#include "ljprofile.h"
#include "ljaccount.h"
#include "ljfriendentry.h"
#include "frienditemdelegate.h"
#include "xmlsettingsmanager.h"
#include "addeditfrienddialog.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	ProfileWidget::ProfileWidget (LJProfile *profile, QWidget *parent)
	: QWidget (parent)
	, Profile_ (profile)
	, FriendsModel_ (new QStandardItemModel (this))
	, CommunitiesModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Ui_.FriendsView_->setModel (FriendsModel_);
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
			Item2FriendGroup_ [item] = group;
			item->setEditable (false);
			FriendsModel_->appendRow (item);
		}

		QStandardItem *withoutGroupItem = 0;
		for (const auto& fr : Profile_->GetFriends ())
		{
			QStandardItem *item = new QStandardItem (fr->GetUserName ());
			QStandardItem *itemName = new QStandardItem (fr->GetFullName ());
			item->setEditable (false);
			itemName->setEditable (false);
			item->setData (fr->GetBGColor ().name (), ItemColorRoles::BackgroundColor);
			item->setData (fr->GetFGColor ().name (), ItemColorRoles::ForegroundColor);

			bool found = false;
			for (const auto& parentItem : Item2FriendGroup_.keys ())
			{
				if (Item2FriendGroup_ [parentItem].RealId_ == fr->GetGroupMask ())
				{
					parentItem->appendRow ({ item, itemName });
					found = true;
				}
			}

			if (!found)
			{
				if (!withoutGroupItem)
				{
					withoutGroupItem = new QStandardItem (tr ("Without group"));
					FriendsModel_->appendRow (withoutGroupItem);
				}

				withoutGroupItem->appendRow ({ item, itemName });
			}
		}

		Ui_.FriendsView_->header ()->setResizeMode (QHeaderView::ResizeToContents);
	}

	void ProfileWidget::FillCommunities (const QStringList& communities)
	{
		for (const auto& community : communities)
		{
			QStandardItem *item = new QStandardItem (community);
			item->setEditable (false);
			CommunitiesModel_->appendRow (item);
		}
	}

	void ProfileWidget::ReFillModels ()
	{
		const LJProfileData& data = Profile_->GetProfileData ();

		FriendsModel_->clear ();
		FriendsModel_->setHorizontalHeaderLabels ({ tr ("UserName"), tr ("FullName") });
		Item2Friend_.clear ();
		Item2FriendGroup_.clear();
		FillFriends (data.FriendGroups_);

		CommunitiesModel_->clear ();
		CommunitiesModel_->setHorizontalHeaderLabels ({ tr ("Name") });
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
		emit coloringItemChanged ();
	}

	void ProfileWidget::on_AddFriend__released ()
	{
		std::unique_ptr<AddEditFriendDialog> aefd (new AddEditFriendDialog (Profile_));
		if (aefd->exec () == QDialog::Rejected)
			return;

		const QString& userName = aefd->GetUserName ();
		const QString& bgcolor = aefd->GetBackgroundColorName ();
		const QString& fgcolor = aefd->GetForegroundColorName ();
		uint realId =  aefd->GetGroupRealId ();

		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		account->AddNewFriend (userName, bgcolor, fgcolor, realId);
	}

	void ProfileWidget::on_EditFriend__released ()
	{
		//TODO
	}

	void ProfileWidget::on_DeleteFriend__released ()
	{
		auto indexList = Ui_.FriendsView_->selectionModel ()->selectedIndexes ();
		if (indexList.isEmpty ())
			return;

		int res = QMessageBox::question (this,
				tr ("Delete friend."),
				tr ("Are you sure to delete selected users from your friends?"
					"<br><i>Note: if you select group all users in this group will be deleted</i>"),
				QMessageBox::Ok | QMessageBox::Cancel,
				QMessageBox::Cancel);

		if (res == QMessageBox::Ok)
		{
			LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
			if (!account)
				return;

			QStringList names;
			for (const auto& index : indexList)
			{
				if (!index.parent ().isValid ())
					for (int i = 0; i < FriendsModel_->rowCount (index); ++i)
						names << FriendsModel_->index (i, Columns::Name, index).data ().toString ();
				else
					names << index.sibling (index.row (), Columns::Name).data ().toString ();
			}
			names.removeDuplicates ();
			account->DeleteFriends (names);
		}
	}

	void ProfileWidget::on_UpdateProfile__released ()
	{
		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;
		account->updateProfile ();
	}

}
}
}


