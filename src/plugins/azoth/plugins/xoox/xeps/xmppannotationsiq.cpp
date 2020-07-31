/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmppannotationsiq.h"
#include <QDomElement>

namespace LC
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

		for (const auto& item : Items_)
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
