/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmppbobiq.h"
#include <QDomElement>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NSBob = "urn:xmpp:bob";

	XMPPBobIq::XMPPBobIq (QXmppIq::Type type)
	: QXmppIq (type)
	, MaxAge_ (86400)
	{
	}

	QString XMPPBobIq::GetCid () const
	{
		return Cid_;
	}

	void XMPPBobIq::SetCid (const QString& cid)
	{
		Cid_ = cid;
	}

	QByteArray XMPPBobIq::GetData () const
	{
		return Data_;
	}

	void XMPPBobIq::SetData (const QByteArray& data)
	{
		Data_ = data;
	}

	QString XMPPBobIq::GetMimeType () const
	{
		return MimeType_;
	}

	void XMPPBobIq::SetMimeType (const QString& type)
	{
		MimeType_ = type;
	}

	int XMPPBobIq::GetMaxAge () const
	{
		return MaxAge_;
	}

	void XMPPBobIq::SetMaxAge (int maxAge)
	{
		MaxAge_ = maxAge;
	}

	bool XMPPBobIq::IsBobIq (const QDomElement& element)
	{
		const auto& dataElement = element.firstChildElement ("data");
		return dataElement.namespaceURI () == NSBob;
	}

	void XMPPBobIq::parseElementFromChild (const QDomElement& element)
	{
		const auto& dataElement = element.firstChildElement ("data");
		Cid_ = dataElement.attribute ("cid");
		MimeType_ = dataElement.attribute ("type");
		MaxAge_ = dataElement.attribute ("max-age", "-1").toInt ();
		Data_ = QByteArray::fromBase64 (dataElement.text ().toLatin1 ());
	}

	void XMPPBobIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
	{
		writer->writeStartElement ("data");
		writer->writeAttribute ("xmlns", NSBob);
		writer->writeAttribute ("cid", Cid_);

		if (!MimeType_.isEmpty ())
			writer->writeAttribute ("type", MimeType_);

		if (MaxAge_ >= 0)
			writer->writeAttribute ("max-age", QString::number (MaxAge_));

		if (!Data_.isEmpty ())
			writer->writeCharacters (Data_.toBase64 ());

		writer->writeEndElement ();
	}
}
}
}
