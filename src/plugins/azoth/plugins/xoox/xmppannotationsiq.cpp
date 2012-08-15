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

#include "xmppannotationsiq.h"
#include <QDomElement>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NSPrivate = "jabber:iq:private";
	const QString NSRosterNotes = "storage:rosternotes";

	XMPPAnnotationsIq::NoteItem::NoteItem ()
	{
	}

	XMPPAnnotationsIq::NoteItem::NoteItem (const QString& jid, const QString& note)
	: Jid_ (jid)
	, Note_ (note)
	{
	}

	QString XMPPAnnotationsIq::NoteItem::GetJid () const
	{
		return Jid_;
	}

	void XMPPAnnotationsIq::NoteItem::SetJid (const QString& jid)
	{
		Jid_ = jid;
	}

	QString XMPPAnnotationsIq::NoteItem::GetNote () const
	{
		return Note_;
	}

	void XMPPAnnotationsIq::NoteItem::SetNote (const QString& note)
	{
		Note_ = note;
	}

	QDateTime XMPPAnnotationsIq::NoteItem::GetCDate () const
	{
		return CDate_;
	}

	void XMPPAnnotationsIq::NoteItem::SetCDate (const QDateTime& cdate)
	{
		CDate_ = cdate;
	}

	QDateTime XMPPAnnotationsIq::NoteItem::GetMDate () const
	{
		return MDate_;
	}

	void XMPPAnnotationsIq::NoteItem::SetMDate (const QDateTime& mdate)
	{
		MDate_ = mdate;
	}

	XMPPAnnotationsIq::XMPPAnnotationsIq ()
	{
	}

	QList<XMPPAnnotationsIq::NoteItem> XMPPAnnotationsIq::GetItems () const
	{
		return Items_;
	}

	void XMPPAnnotationsIq::SetItems (const QList<NoteItem>& items)
	{
		Items_ = items;
	}

	void XMPPAnnotationsIq::parseElementFromChild (const QDomElement& element)
	{
		const auto& storage = element.firstChildElement ("query").firstChildElement ("storage");
		auto note = storage.firstChildElement ("note");
		while (!note.isNull ())
		{
			NoteItem item (note.attribute ("jid"), note.text ());

			if (note.hasAttribute ("cdate"))
				item.SetCDate (QDateTime::fromString (note.attribute ("cdate"), Qt::ISODate));
			if (note.hasAttribute ("mdate"))
				item.SetMDate (QDateTime::fromString (note.attribute ("mdate"), Qt::ISODate));

			Items_ << item;

			note = note.nextSiblingElement ("note");
		}
	}

	void XMPPAnnotationsIq::toXmlElementFromChild (QXmlStreamWriter *writer) const
	{
		writer->writeStartElement ("query");
		writer->writeAttribute ("xmlns", NSPrivate);
		writer->writeStartElement ("storage");
		writer->writeAttribute ("xmlns", NSRosterNotes);

		Q_FOREACH (const auto& item, Items_)
		{
			writer->writeStartElement ("note");
			writer->writeAttribute ("jid", item.GetJid ());

			if (item.GetCDate ().isValid ())
				writer->writeAttribute ("cdate", item.GetCDate ().toString (Qt::ISODate));

			if (item.GetMDate ().isValid ())
				writer->writeAttribute ("mdate", item.GetMDate ().toString (Qt::ISODate));

			writer->writeCharacters (item.GetNote ());
			writer->writeEndElement ();
		}

		writer->writeEndElement ();
		writer->writeEndElement ();
	}
}
}
}
