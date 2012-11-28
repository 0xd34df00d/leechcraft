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

#include <functional>
#include <QObject>
#include <QQueue>
#include <QPair>
#include <QDomElement>
#include <QNetworkRequest>
#include "core.h"
#include "profiletypes.h"
#include "ljaccount.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJFriendEntry;

	class LJXmlRPC : public QObject
	{
		Q_OBJECT

		LJAccount *Account_;
		QQueue<std::function<void (const QString&)>> ApiCallQueue_;

		const int BitMaskForFriendsOnlyComments_;
	public:
		LJXmlRPC (LJAccount *acc, QObject *parent = 0);

		void Validate (const QString& login, const QString& pass);

		void AddNewFriend (const QString& username,
				const QString& bgcolor, const QString& fgcolor, uint groupId);
		void DeleteFriend (const QString& username);

		void AddGroup (const QString& name, bool isPublic, int id);
		void DeleteGroup (int id);

		void UpdateProfileInfo ();
		void Submit (const LJEvent& event);
	private:
		void GenerateChallenge () const;
		void ValidateAccountData (const QString& login,
				const QString& pass, const QString& challenge);
		void RequestFriendsInfo (const QString& login,
				const QString& pass, const QString& challenge);
		void ParseForError (const QByteArray& content);
		void ParseFriends (const QDomDocument& doc);

		void AddNewFriendRequest (const QString& username,
				const QString& bgcolor, const QString& fgcolor,
				int groupId, const QString& challenge);
		void DeleteFriendRequest (const QString& usernames,
				const QString& challenge);

		void AddGroupRequest (const QString& name, bool isPublic, int id,
				const QString& challenge);
		void DeleteGroupRequest (int id, const QString& challenge);

		void PostEventRequest (const LJEvent& event, const QString& challenge);
	private slots:
		void handleChallengeReplyFinished ();
		void handleValidateReplyFinished ();
		void handleRequestFriendsInfoFinished ();
		void handleAddNewFriendReplyFinished ();
		void handleReplyWithProfileUpdate ();
		void handlePostEventReplyFinished ();

	signals:
		void validatingFinished (bool success);
		void profileUpdated (const LJProfileData& profile);
		void error (int code, const QString& msg);
		void entryPosted ();
	};
}
}
}
