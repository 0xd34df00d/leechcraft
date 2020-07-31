/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmppcaptchaiq.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NSCaptcha = "urn:xmpp:captcha";

	XMPPCaptchaIq::XMPPCaptchaIq (QXmppIq::Type type)
	: QXmppIq (type)
	{
	}

	QXmppDataForm XMPPCaptchaIq::GetDataForm () const
	{
		return DataForm_;
	}

	void XMPPCaptchaIq::SetDataForm (const QXmppDataForm& dataForm)
	{
		DataForm_ = dataForm;
	}

	void XMPPCaptchaIq::toXmlElementFromChild (QXmlStreamWriter *writer) const
	{
		writer->writeStartElement ("captcha");
		writer->writeAttribute ("xmlns", NSCaptcha);
		DataForm_.toXml (writer);
		writer->writeEndElement ();
	}
}
}
}
