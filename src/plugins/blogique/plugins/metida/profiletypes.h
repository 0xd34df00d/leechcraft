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

#include <QUrl>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include "ljfriendentry.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	struct LJFriendGroup
	{
		bool Public_;
		QString Name_;
		uint Id_;
		uint SortOrder_;
		uint RealId_;
	};

	struct LJMood
	{
		qint64 Parent_;
		qint64 Id_;
		QString Name_;
	};

	struct LJProfileData
	{
		QUrl AvatarUrl_;
		qint64 UserId_;
		qint64 Caps_;
		QList<LJFriendGroup> FriendGroups_;
		QList<LJMood> Moods_;
		QStringList Communities_;
		QString FullName_;
		QList<LJFriendEntry_ptr> Friends_;
		QStringList AvatarsID_;
		QList<QUrl> AvatarsUrls_;
	};

	namespace LJParserTypes
	{
		class LJParseProfileEntry
		{
			QString Name_;
			QVariantList ValueList_;

		public:
			LJParseProfileEntry ();
			LJParseProfileEntry (const QString& name,
					const QVariantList& value);
			QString Name () const;
			QVariantList Value () const;

			bool ValueToBool () const;
			QString ValueToString () const;
			qint64 ValueToLongLong () const;
			int ValueToInt () const;
			QUrl ValueToUrl () const;
		};
	}

	namespace LJInbox
	{
		enum class MessageState
		{
			Read,
			UnRead
		};

		enum MessageType
		{
			NoType = 0,
			Friended,
			Birthday,
			CommunityInvite,
			CommunityJoinApprove,
			CommunityJoinReject,
			CommunityJoinRequest,
			Defriended,
			InvitedFriendJoins,
			JournalNewComment,
			JournalNewEntry,
			NewUserpic,
			NewVGift,
			OfficialPost,
			PermSale,
			PollVote,
			SupOfficialPost,
			UserExpunged,
			UserMessageRecvd,
			UserMessageSent,
			UserNewComment,
			UserNewEntry
		};

		struct Message
		{
			int Id_;
			QDateTime When_;
			MessageState State_;
			int Type_;
			QString TypeString_;
			QString ExtendedSubject_;
			QString ExtendedText_;
			int ExternalId_;

			Message ()
			: Id_(-1)
			, State_ (MessageState::UnRead)
			, Type_ (-1)
			, ExternalId_ (-1)
			{};
		};

		struct MessageNewComment : public Message
		{
			QString Journal_;
			QString Action_;
			QUrl Url_;
			QUrl ReplyUrl_;
			QString AuthorName_;
			QString Subject_;
		};

		struct MessageRecvd : public Message
		{
			QString From_;
			QUrl PictureUrl_;
			QString Subject_;
			QString Body_;
			int MessageId_;
			int ParentId_;

			MessageRecvd ()
			: Message ()
			, MessageId_ (-1)
			, ParentId_ (-1)
			{};
		};

		struct MessageSent : public Message
		{
			QString To_;
			QUrl PictureUrl_;
			QString Subject_;
			QString Body_;
		};
	}

	QDataStream& operator<< (QDataStream& out, const LJFriendGroup& group);
	QDataStream& operator>> (QDataStream& in, LJFriendGroup& group);
	QDataStream& operator<< (QDataStream& out, const LJMood& mood);
	QDataStream& operator>> (QDataStream& in, LJMood& mood);
	QDataStream& operator<< (QDataStream& out, const LJProfileData& data);
	QDataStream& operator>> (QDataStream& in, LJProfileData& data);
}
}
}

Q_DECLARE_METATYPE (LeechCraft::Blogique::Metida::LJParserTypes::LJParseProfileEntry)
Q_DECLARE_METATYPE (LeechCraft::Blogique::Metida::LJProfileData)
