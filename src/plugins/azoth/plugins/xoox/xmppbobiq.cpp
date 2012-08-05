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

#include "xmppbobiq.h"
#include <QDomElement>
#include <QXmppConstants.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
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
		return dataElement.namespaceURI () == ns_bob;
	}

	void XMPPBobIq::parseElementFromChild(const QDomElement &element)
	{
		const auto& dataElement = element.firstChildElement ("data");
		Cid_ = dataElement.attribute ("cid");
		MimeType_ = dataElement.attribute ("type");
		MaxAge_ = dataElement.attribute ("max-age", "-1").toInt ();
		Data_ = QByteArray::fromBase64 (dataElement.text ().toAscii ());
	}

	void XMPPBobIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
	{
		writer->writeStartElement ("data");
		writer->writeAttribute ("xmlns", ns_bob);
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
