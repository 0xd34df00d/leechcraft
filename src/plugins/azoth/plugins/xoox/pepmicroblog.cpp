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

#include "pepmicroblog.h"
#include <QUuid>
#include <QDomElement>
#include <QXmppElement.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsMicroblog = "urn:xmpp:microblog:0";
	const QString NsAtom = "http://www.w3.org/2005/Atom";
	const QString NsThread = "http://purl.org/syndication/thread/1.0";

	QString PEPMicroblog::GetNodeString ()
	{
		return NsMicroblog;
	}

	PEPMicroblog::PEPMicroblog ()
	: EventID_ (QUuid::createUuid ().toString ().remove ('{').remove ('}'))
	{
	}

	namespace
	{
		QXmppElement GetAuthorElem (const QString& name, const QString& uri)
		{
			QXmppElement nameElem;
			nameElem.setTagName ("name");
			nameElem.setValue (name);

			QXmppElement uriElem;
			uriElem.setTagName ("uri");
			uriElem.setValue (uri);

			QXmppElement author;
			author.setTagName ("author");
			author.appendChild (nameElem);
			author.appendChild (uriElem);

			return author;
		}
	}

	QXmppElement PEPMicroblog::ToXML () const
	{
		QXmppElement entry;
		entry.setTagName ("entry");
		entry.setAttribute ("xmlns", NsAtom);

		QXmppElement id;
		id.setTagName ("id");
		id.setValue (EventID_);
		entry.appendChild (id);

		QXmppElement source;
		source.setTagName ("source");
		source.appendChild (GetAuthorElem (AuthorName_, AuthorURI_));
		entry.appendChild (source);

		Q_FOREACH (const auto& key, Contents_.keys ())
		{
			QXmppElement cElem;
			cElem.setTagName ("content");
			cElem.setAttribute ("type", key);
			cElem.setValue (Contents_ [key]);
			entry.appendChild (cElem);
		}

		Q_FOREACH (const auto& linkInfo, Alternates_)
		{
			QXmppElement link;
			link.setTagName ("link");
			link.setAttribute ("rel", "alternate");
			if (!linkInfo.Type_.isEmpty ())
				link.setAttribute ("type", linkInfo.Type_);
			link.setAttribute ("href", linkInfo.Href_);
			entry.appendChild (link);
		}

		Q_FOREACH (const auto& repInfo, InReplyTo_)
		{
			QXmppElement elem;
			elem.setTagName ("in-reply-to");
			elem.setAttribute ("xmlns", NsThread);

			if (!repInfo.Type_.isNull ())
				elem.setAttribute ("type", repInfo.Type_);
			elem.setAttribute ("ref", repInfo.Ref_);
			elem.setAttribute ("href", repInfo.Href_);

			entry.appendChild (elem);
		}

		QXmppElement published;
		published.setTagName ("published");
		published.setValue (Published_.toString (Qt::ISODate));
		entry.appendChild (published);

		if (!Updated_.isNull ())
		{
			QXmppElement updated;
			updated.setTagName ("updated");
			updated.setValue (Updated_.toString (Qt::ISODate));
			entry.appendChild (updated);
		}

		QXmppElement result;
		result.setTagName ("item");
		result.setAttribute ("id", EventID_);
		result.appendChild (entry);
		return result;
	}

	void PEPMicroblog::Parse (const QDomElement& elem)
	{
		EventID_ = elem.attribute ("id");

		const QDomElement& entry = elem.firstChildElement ("entry");
		if (entry.namespaceURI () != NsAtom)
			return;

		const QDomElement& author = entry
				.firstChildElement ("source")
				.firstChildElement ("author");
		AuthorName_ = author.firstChildElement ("author").text ();
		AuthorURI_ = author.firstChildElement ("uri").text ();

		QDomElement content = entry.firstChildElement ("content");
		while (!content.isNull ())
		{
			Contents_ [content.attribute ("type")] = content.text ();
			content = content.nextSiblingElement ("content");
		}

		QDomElement link = entry.firstChildElement ("link");
		while (!link.isNull ())
		{
			if (link.attribute ("rel") == "alternate")
			{
				AlternateLink l =
				{
					link.attribute ("type"),
					link.attribute ("href")
				};

				Alternates_ << l;
			}

			link = link.nextSiblingElement ("link");
		}

		QDomElement inReplyTo = entry.firstChildElement ("in-reply-to");
		while (!inReplyTo.isNull ())
		{
			ReplyInfo info =
			{
				inReplyTo.attribute ("type"),
				inReplyTo.attribute ("ref"),
				inReplyTo.attribute ("href")
			};
			InReplyTo_ << info;

			inReplyTo = inReplyTo.nextSiblingElement ("in-reply-to");
		}

		Published_ = QDateTime::fromString (entry.firstChildElement ("published").text (), Qt::ISODate);
		Updated_ = QDateTime::fromString (entry.firstChildElement ("updated").text (), Qt::ISODate);
	}

	QString PEPMicroblog::Node () const
	{
		return GetNodeString ();
	}

	PEPEventBase* PEPMicroblog::Clone () const
	{
		return new PEPMicroblog (*this);
	}

	QString PEPMicroblog::GetEventID () const
	{
		return EventID_;
	}

	QString PEPMicroblog::GetAuthorName () const
	{
		return AuthorName_;
	}

	void PEPMicroblog::SetAuthorName (const QString& name)
	{
		AuthorName_ = name;
	}

	QString PEPMicroblog::GetAuthorURI () const
	{
		return AuthorURI_;
	}

	void PEPMicroblog::SetAuthorURI (const QString& uri)
	{
		AuthorURI_ = uri;
	}

	QString PEPMicroblog::GetContent (const QString& type) const
	{
		return Contents_ [type];
	}

	void PEPMicroblog::SetContent (const QString& type, const QString& content)
	{
		Contents_ [type] = content;
	}

	QDateTime PEPMicroblog::GetPublishedDate () const
	{
		return Published_;
	}

	void PEPMicroblog::SetPublishedDate (const QDateTime& date)
	{
		Published_ = date;
	}

	QDateTime PEPMicroblog::GetUpdatedDate () const
	{
		return Updated_;
	}

	void PEPMicroblog::SetUpdatedDate (const QDateTime& date)
	{
		Updated_ = date;
	}

	PEPMicroblog::AlternateLinks_t PEPMicroblog::GetAlternateLinks () const
	{
		return Alternates_;
	}

	void PEPMicroblog::SetAlternateLinks (const AlternateLinks_t& links)
	{
		Alternates_ = links;
	}

	void PEPMicroblog::AddAlternateLink (const AlternateLink& link)
	{
		Alternates_ << link;
	}

	PEPMicroblog::ReplyInfos_t PEPMicroblog::GetInReplyTos () const
	{
		return InReplyTo_;
	}

	void PEPMicroblog::SetInReplyTos (const ReplyInfos_t& infos)
	{
		InReplyTo_ = infos;
	}

	void PEPMicroblog::AddInReplyTos (const ReplyInfo& info)
	{
		InReplyTo_ << info;
	}
}
}
}
