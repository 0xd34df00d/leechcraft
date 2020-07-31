/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmppcaptchamanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include "xmppcaptchaiq.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NSCaptcha = "urn:xmpp:captcha";

	bool XMPPCaptchaManager::handleStanza (const QDomElement& stanza)
	{
		if (stanza.tagName () != "message")
			return false;

		const auto& captchaStanza = stanza.firstChildElement ("captcha");

		if (captchaStanza.namespaceURI () != NSCaptcha)
			return false;

		const auto& dataFormStanza = captchaStanza.firstChildElement ("x");
		if (dataFormStanza.isNull ())
			return false;

		QXmppDataForm dataForm;
		dataForm.parse (dataFormStanza);

		if (dataForm.isNull ())
			return false;

		emit captchaFormReceived (stanza.attribute ("from"), dataForm);
		return true;
	}

	QString XMPPCaptchaManager::SendResponse (const QString& to, const QXmppDataForm& form)
	{
		XMPPCaptchaIq request;
		request.setType (QXmppIq::Set);
		request.setTo (to);
		request.SetDataForm (form);
		if(client ()->sendPacket (request))
			return request.id ();
		else
			return QString ();
	}

}
}
}
