/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/core/icoreproxyfwd.h>
#include <interfaces/blogique/iprofilewidget.h>
#include "profiletypes.h"
#include "ui_profilewidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJFriendEntry;
	class LJProfile;
	class FriendsProxyModel;

	class ProfileWidget final : public QWidget
							  , public IProfileWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blogique::IProfileWidget)

		Ui::ProfileWidget Ui_;

		enum Columns
		{
			Name
		};
		
		LJProfile * const Profile_;
		const ICoreProxy_ptr Proxy_;

		QStandardItemModel * const FriendsModel_;
		FriendsProxyModel * const FriendsProxyModel_;
		QStandardItemModel * const GroupsModel_;
		QStandardItemModel * const FriendsInGroupModel_;
		QStandardItemModel * const FriendsNotInGroupModel_;
		QStandardItemModel * const CommunitiesModel_;
		QHash<QStandardItem*, LJFriendGroup> Item2FriendGroup_;
		QHash<QStandardItem*, LJFriendEntry_ptr> Item2Friend_;
		QMap<LJFriendEntry_ptr, QStandardItem*> Friend2Item_;
		QHash<QString, LJFriendEntry_ptr> Username2Friend_;

	public:
		ProfileWidget (LJProfile *profile, const ICoreProxy_ptr& proxy, QWidget *parent = nullptr);
	private:
		void RereadProfileData ();
		void FillFriends (const QList<LJFriendEntry_ptr>& friends);
		void FillGroups (const QList<LJFriendGroup>& friendGroups);
		void FillCommunities (const QStringList& communities);
		void ReFillModels ();

	public slots:
		void updateProfile () override;
	private slots:
		void on_ColoringFriendsList__toggled (bool toggle);
		void on_Add__released ();
		void on_Edit__released ();
		void on_Delete__released ();
		void on_UpdateProfile__released ();
		void on_Groups__clicked (const QModelIndex& index);
		void on_AddUserToGroup__released ();
		void on_RemoveUserFromGroup__released ();
		void on_SendMessage__released ();
		void handleUserGroupChanged (const QString& username,
				const QString& bgColor, const QString& fgColor, int groupId);
		void handleFriendFilterTextChanged (const QString& text);
		void handleReadJournal ();
		void handleSendMessage ();
		void handleReadCommunity ();
		void handleFriendsViewDoubleClicked (const QModelIndex& index);
		void handleCommunitiesViewDoubleClicked (const QModelIndex& index);
		
		void addNewGroup ();
		void deleteGroup ();
		void editGroup ();
		
	signals:
		void coloringItemChanged ();
	};
}
}
}
