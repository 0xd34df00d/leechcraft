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
#include <interfaces/blogique/iprofilewidget.h>
#include "profiletypes.h"
#include "ui_profilewidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJFriendEntry;
	class LJProfile;

	class ProfileWidget : public QWidget
						, public IProfileWidget
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Blogique::IProfileWidget)

		Ui::ProfileWidget Ui_;
		LJProfile *Profile_;
		QStandardItemModel *FriendsModel_;
		QStandardItemModel *CommunitiesModel_;
		QHash<QStandardItem*, LJFriendGroup> Item2FriendGroup_;
		QHash<QStandardItem*, LJFriendEntry> Item2Friend_;
	public:
		ProfileWidget (LJProfile *profile, QWidget *parent = 0);
	private:
		void RereadProfileData ();
		void FillFriends (const QList<LJFriendGroup>& groups);
		void FillCommunities (const QStringList& communities);

	public slots:
		void updateProfile ();
	private slots:
		void on_ColoringFriendsList__toggled (bool toggle);

	signals:
		void coloringItemChanged ();
	};
}
}
}
