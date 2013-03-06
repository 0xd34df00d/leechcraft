/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "xmppcaptchamanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include "xmppcaptchaiq.h"

namespace LeechCraft
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
