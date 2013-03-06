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
#include <QNetworkReply>
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
		const int MaxGetEventsCount_;

		QHash<QNetworkReply*, int> Reply2Skip_;

		enum class RequestType
		{
			Update,
			Post
		};
		QHash<QNetworkReply*, RequestType> Reply2RequestType_;

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
		void BackupEvents ();
		void GetLastEvents (int count);
		void GetChangedEvents (const QDateTime& dt);
		void GetEventsByDate (const QDate& date);

		void RemoveEvent (const LJEvent& event);
		void UpdateEvent (const LJEvent& event);

		void RequestStatistics ();

		void RequestInbox ();
	private:
		void GenerateChallenge () const;
		void ValidateAccountData (const QString& login,
				const QString& pass, const QString& challenge);
		void RequestFriendsInfo (const QString& login,
				const QString& pass, const QString& challenge);
		void AddNewFriendRequest (const QString& username,
				const QString& bgcolor, const QString& fgcolor,
				int groupId, const QString& challenge);
		void DeleteFriendRequest (const QString& usernames,
				const QString& challenge);

		void AddGroupRequest (const QString& name, bool isPublic, int id,
				const QString& challenge);
		void DeleteGroupRequest (int id, const QString& challenge);

		void PostEventRequest (const LJEvent& event, const QString& challenge);
		void RemoveEventRequest (const LJEvent& event, const QString& challenge);
		void UpdateEventRequest (const LJEvent& event, const QString& challenge);

		void BackupEventsRequest (int skip, const QString& challenge);

		void GetLastEventsRequest (int count, const QString& challenge);
		void GetChangedEventsRequest (const QDateTime& dt, const QString& challenge);
		void GetEventsByDateRequest (const QDate& date, const QString& challenge);
		void GetParticularEventRequest (int id, RequestType prt,
				const QString& challenge);

		void BlogStatisticsRequest (const QString& challenge);

		void InboxRequest (const QString& challenge);

		void ParseForError (const QByteArray& content);
		void ParseFriends (const QDomDocument& doc);

		QList<LJEvent> ParseFullEvents (const QDomDocument& doc);

		QMap<QDate, int> ParseStatistics (const QDomDocument& doc);


	private slots:
		void handleChallengeReplyFinished ();
		void handleValidateReplyFinished ();
		void handleRequestFriendsInfoFinished ();
		void handleAddNewFriendReplyFinished ();
		void handleReplyWithProfileUpdate ();
		void handlePostEventReplyFinished ();
		void handleBackupEventsReplyFinished ();
		void handleGotEventsReplyFinished ();
		void handleRemoveEventReplyFinished ();
		void handleUpdateEventReplyFinished ();
		void handleGetParticularEventReplyFinished ();
		void handleBlogStatisticsReplyFinished ();
		void handleInboxReplyFinished ();

		void handleNetworkError (QNetworkReply::NetworkError error);

	signals:
		void validatingFinished (bool success);
		void profileUpdated (const LJProfileData& profile);
		void error (int code, const QString& msg);
		void networkError (int code, const QString& msg);

		void eventPosted (const QList<LJEvent>& events);
		void eventUpdated (const QList<LJEvent>& events);
		void eventRemoved (int itemId);

		void gotEvents2Backup (const QList<LJEvent>& events);
		void gettingEvents2BackupFinished ();

		void gotEvents (const QList<LJEvent>& events);

		void gotStatistics (const QMap<QDate, int>& statistics);
	};
}
}
}
