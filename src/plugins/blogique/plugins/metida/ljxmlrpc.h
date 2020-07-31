/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QQueue>
#include <QPair>
#include <QDomElement>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <interfaces/core/icoreproxyfwd.h>
#include "profiletypes.h"
#include "ljaccount.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJFriendEntry;

	class LJXmlRPC : public QObject
	{
		Q_OBJECT

		LJAccount * const Account_;
		const ICoreProxy_ptr Proxy_;

		QQueue<std::function<void (const QString&)>> ApiCallQueue_;

		QHash<QNetworkReply*, int> Reply2Skip_;
		QHash<QNetworkReply*, Filter> Reply2Filter_;
		QHash<QNetworkReply*, QDate> Reply2Date_;

		enum class RequestType
		{
			Update,
			Post,
			RecentComments,
			Tags
		};

		QHash<QNetworkReply*, RequestType> Reply2RequestType_;
		QMap<QPair<int, int>, LJCommentEntry> Id2CommentEntry_;

	public:
		LJXmlRPC (LJAccount *acc, const ICoreProxy_ptr& proxy, QObject *parent = 0);

		void Validate (const QString& login, const QString& pass);

		void AddNewFriend (const QString& username,
				const QString& bgcolor, const QString& fgcolor, uint groupMask);
		void DeleteFriend (const QString& username);

		void AddGroup (const QString& name, bool isPublic, int id);
		void DeleteGroup (int id);

		void UpdateProfileInfo ();

		void Preview (const LJEvent& event);
		void Submit (const LJEvent& event);
		void GetEventsWithFilter (const Filter& filter);
		void GetLastEvents (int count);
		void GetMultiplyEvents (const QList<int>& ids, RequestType rt);
		void GetParticularEvent (int id, RequestType rt);
		void GetChangedEvents (const QDateTime& dt);
		void GetEventsByDate (const QDate& date, int skip = 0);

		void RemoveEvent (const LJEvent& event);
		void UpdateEvent (const LJEvent& event);

		void RequestStatistics ();

		void RequestLastInbox ();
		void SetMessagesAsRead (const QList<int>& ids);
		void SendMessage (const QStringList& addresses, const QString& subject,
				const QString& text);

		void RequestRecentCommments ();
		void DeleteComment (qint64 id, bool deleteThread = false);
		void AddComment (const CommentEntry& comment);

		void RequestTags ();

	private:
		std::shared_ptr<void> MakeRunnerGuard ();
		void CallNextFunctionFromQueue ();
		void GenerateChallenge () const;
		void ValidateAccountData (const QString& login,
				const QString& pass, const QString& challenge);
		void RequestFriendsInfo (const QString& login,
				const QString& pass, const QString& challenge);
		void AddNewFriendRequest (const QString& username,
				const QString& bgcolor, const QString& fgcolor,
				int groupMask, const QString& challenge);
		void DeleteFriendRequest (const QString& usernames,
				const QString& challenge);

		void AddGroupRequest (const QString& name, bool isPublic, int id,
				const QString& challenge);
		void DeleteGroupRequest (int id, const QString& challenge);

		void PreviewEventRequest (const LJEvent& event, const QString& challenge);
		void PostEventRequest (const LJEvent& event, const QString& challenge);
		void RemoveEventRequest (const LJEvent& event, const QString& challenge);
		void UpdateEventRequest (const LJEvent& event, const QString& challenge);

		void BackupEventsRequest (const Filter& filter, const QString& challenge);

		void GetLastEventsRequest (int count, const QString& challenge);
		void GetChangedEventsRequest (const QDateTime& dt, const QString& challenge);
		void GetEventsByDateRequest (const QDate& date, int skip = 0,
				const QString& challenge = QString ());
		void GetParticularEventRequest (int id, RequestType prt,
				const QString& challenge);
		void GetMultipleEventsRequest (const QList<int>& ids, RequestType rt,
				const QString& challenge);

		void BlogStatisticsRequest (const QString& challenge);

		void InboxRequest (const QString& challenge);
		void SetMessageAsReadRequest (const QList<int>& ids, const QString& challenge);
		void SendMessageRequest (const QStringList& addresses, const QString& subject,
				const QString& text, const QString& challenge);

		void RecentCommentsRequest (const QString& challenge);
		void DeleteCommentRequest (qint64 id, bool deleteThread, const QString& challenge);
		void AddCommentRequest (const CommentEntry& comment, const QString& challenge);

		void GetUserTagsRequest (const QString& challenge);

		void ParseForError (const QByteArray& content);
		void ParseFriends (const QDomDocument& doc);

	private slots:
		void handleChallengeReplyFinished ();
		void handleValidateReplyFinished ();
		void handleRequestFriendsInfoFinished ();
		void handleAddNewFriendReplyFinished ();
		void handleReplyWithProfileUpdate ();
		void handlePreviewEventReplyFinished ();
		void handlePostEventReplyFinished ();
		void handleBackupEventsReplyFinished ();
		void handleGotEventsReplyFinished ();
		void handleGotEventsByDateReplyFinished ();
		void handleRemoveEventReplyFinished ();
		void handleUpdateEventReplyFinished ();
		void handleGetParticularEventReplyFinished ();
		void handleGetMultipleEventsReplyFinished ();
		void handleBlogStatisticsReplyFinished ();
		void handleInboxReplyFinished ();
		void handleMessagesSetAsReadFinished ();
		void handleSendMessageRequestFinished ();
		void handleRecentCommentsReplyFinished ();
		void handleDeleteCommentReplyFinished ();
		void handleAddCommentReplyFinished ();
		void handleGetUserTagsReplyFinished ();

		void handleNetworkError (QNetworkReply::NetworkError error);

	signals:
		void validatingFinished (bool success);
		void profileUpdated (const LJProfileData& profile);
		void error (int code, const QString& msg,
				const QString& localizedMsg);
		void networkError (int code, const QString& msg);

		void eventPosted (const QList<LJEvent>& events);
		void eventUpdated (const QList<LJEvent>& events);
		void eventRemoved (int itemId);

		void gotFilteredEvents (const QList<LJEvent>& events);
		void gettingFilteredEventsFinished ();

		void gotEvents (const QList<LJEvent>& events);

		void gotStatistics (const QMap<QDate, int>& statistics);

		void unreadMessagesIds (const QList<int>& unreadIds);
		void messagesRead ();
		void messageSent ();

		void gotRecentComments (const QList<LJCommentEntry>& comments);
		void commentsDeleted (const QList<qint64>& ids);
		void commentSent (const QUrl& url);
		
		void gotTags (const QHash<QString, int>& tags);
	};
}
}
}
