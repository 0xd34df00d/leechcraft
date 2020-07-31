/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "pepeventbase.h"
#include <QString>
#include <QDateTime>
#include <QPair>
#include <QMap>
#include <interfaces/azoth/isupportmicroblogs.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PEPMicroblog : public PEPEventBase
	{
		QString EventID_;

		QString AuthorName_;
		QString AuthorURI_;

		QMap<QString, QString> Contents_;

		QDateTime Published_;
		QDateTime Updated_;
	public:
		struct AlternateLink
		{
			QString Type_;
			QString Href_;
		};
		typedef QList<AlternateLink> AlternateLinks_t;

		struct ReplyInfo
		{
			QString Type_;
			QString Ref_;
			QString Href_;
		};
		typedef QList<ReplyInfo> ReplyInfos_t;
	private:
		AlternateLinks_t Alternates_;

		ReplyInfos_t InReplyTo_;
	public:
		static QString GetNodeString ();

		PEPMicroblog ();
		PEPMicroblog (const Post&);

		QXmppElement ToXML () const override;
		void Parse (const QDomElement&) override;
		QString Node () const override;

		PEPEventBase* Clone () const override;

		operator Post () const;

		QString GetEventID () const override;

		QString GetAuthorName () const;
		void SetAuthorName (const QString&);
		QString GetAuthorURI () const;
		void SetAuthorURI (const QString&);

		QString GetContent (const QString&) const;
		void SetContent (const QString& type, const QString& content);

		QDateTime GetPublishedDate () const;
		void SetPublishedDate (const QDateTime&);

		QDateTime GetUpdatedDate () const;
		void SetUpdatedDate (const QDateTime&);

		AlternateLinks_t GetAlternateLinks () const;
		void SetAlternateLinks (const AlternateLinks_t&);
		void AddAlternateLink (const AlternateLink&);

		ReplyInfos_t GetInReplyTos () const;
		void SetInReplyTos (const ReplyInfos_t&);
		void AddInReplyTos (const ReplyInfo&);
	};
}
}
}
