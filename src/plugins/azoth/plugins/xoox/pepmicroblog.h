/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "pepeventbase.h"
#include <QString>
#include <QDateTime>
#include <QPair>
#include <QMap>

namespace LeechCraft
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

		QXmppElement ToXML () const;
		void Parse (const QDomElement&);
		QString Node () const;

		PEPEventBase* Clone () const;

		QString GetEventID () const;

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
