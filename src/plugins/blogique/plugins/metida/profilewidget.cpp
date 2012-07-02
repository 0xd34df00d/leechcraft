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
#include <util/util.h>
#include "ljprofile.h"
#include "ljaccount.h"
#include "ljfriendentry.h"
#include "frienditemdelegate.h"
#include "xmlsettingsmanager.h"

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
		FriendsModel_->setHorizontalHeaderLabels ({ tr ("Nick") });
		FriendItemDelegate *friendDelegate = new FriendItemDelegate (Ui_.FriendsView_);
		connect (this,
				SIGNAL (coloringItemChanged ()),
				friendDelegate,
				SLOT (handleColoringItemChanged ()));
		Ui_.FriendsView_->setItemDelegate (friendDelegate);

		Ui_.CommunitiesView_->setModel (CommunitiesModel_);
		CommunitiesModel_->setHorizontalHeaderLabels ({ tr ("Name") });

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

		FillFriends (data.FriendGroups_);
		FillCommunities (data.Communities_);
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

		for (const auto& fr : Profile_->GetFriends ())
		{
			QStandardItem *item = new QStandardItem (fr->GetUserName ());
			item->setEditable (false);
			item->setData (fr->GetBGColor ().name (), ItemColorRoles::BackgroundColor);
			item->setData (fr->GetFGColor ().name (), ItemColorRoles::ForegroundColor);

			for (const auto& parentItem : Item2FriendGroup_.keys ())
				if (Item2FriendGroup_ [parentItem].RealId_ == fr->GetGroupMask ())
					parentItem->appendRow (item);
		}
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

}
}
}


