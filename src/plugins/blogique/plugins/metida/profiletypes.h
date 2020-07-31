/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QUrl>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include "ljfriendentry.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	struct LJFriendGroup
	{
		bool Public_;
		QString Name_;
		uint Id_ =  0;
		uint SortOrder_ =  0;
		uint RealId_ =  0;
	};

	struct LJMood
	{
		qint64 Parent_ =  0;
		qint64 Id_ =  0;
		QString Name_;
	};

	struct LJProfileData
	{
		QUrl AvatarUrl_;
		qint64 UserId_ =  0;
		qint64 Caps_ =  0;
		QList<LJFriendGroup> FriendGroups_;
		QList<LJMood> Moods_;
		QStringList Communities_;
		QString FullName_;
		QList<LJFriendEntry_ptr> Friends_;
		QStringList AvatarsID_;
		QList<QUrl> AvatarsUrls_;
		QHash<QString, int> Tags_;
	};

	enum class CommentState
	{
		Frozen,
		Secure,
		Active,
		Deleted
	};

	struct LJCommentEntry
	{
		int NodeId_ =  -1;
		QString Subject_;
		int PosterId_ =  -1;
		CommentState State_ = CommentState::Active;
		int ReplyId_ =  -1;
		int ParentReplyId_ =  -1;
		QString PosterName_;
		QString Text_;
		QDateTime PostingDate_;
		QString NodeSubject_;
		QUrl NodeUrl_;
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

	QDataStream& operator<< (QDataStream& out, const LJFriendGroup& group);
	QDataStream& operator>> (QDataStream& in, LJFriendGroup& group);
	QDataStream& operator<< (QDataStream& out, const LJMood& mood);
	QDataStream& operator>> (QDataStream& in, LJMood& mood);
	QDataStream& operator<< (QDataStream& out, const LJProfileData& data);
	QDataStream& operator>> (QDataStream& in, LJProfileData& data);
}
}
}

Q_DECLARE_METATYPE (LC::Blogique::Metida::LJParserTypes::LJParseProfileEntry)
Q_DECLARE_METATYPE (LC::Blogique::Metida::LJProfileData)
