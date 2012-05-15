/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "ljxmlrpc.h"
#include "core.h"
#include <QDomDocument>
#include <QtDebug>
#include <QCryptographicHash>
#include <QNetworkReply>
#include <QXmlQuery>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJXmlRPC::LJXmlRPC (QObject *parent)
	: QObject (parent)
	{
	}

	void LJXmlRPC::Validate (const QString& login, const QString& password)
	{
		ApiCallQueue_ << [login, password, this] (const QString& challenge)
				{ValidateAccountData (login, password, challenge);};
		GenerateChallenge ();
	}

	void LJXmlRPC::GenerateChallenge () const
	{
		QByteArray output;
		QDomDocument document ("GenerateChallenge");
		QDomElement methodCall = document.createElement ("methodCall");
		document.appendChild (methodCall);

		QDomElement methodName = document.createElement ("methodName");
		methodCall.appendChild (methodName);
		QDomText methodNameText = document.createTextNode ("LJ.XMLRPC.getchallenge");
		methodName.appendChild (methodNameText);

		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleChallengeReplyFinished ()));
	}

	void LJXmlRPC::ValidateAccountData (const QString& login,
			const QString& password, const QString& challenge)
	{
		QDomDocument document ("ValidateRequest");
		auto result = GetStartPart ("LJ.XMLRPC.login", document);
		document.appendChild (result.first);
		const QByteArray passwordHash = QCryptographicHash::hash (password.toUtf8 (),
				QCryptographicHash::Md5).toHex ();

		QString hpassword = QCryptographicHash::hash ((challenge + passwordHash).toUtf8 (),
				QCryptographicHash::Md5).toHex ();
		result.second.appendChild (GetMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetMemberElement ("username", "string",
				login, document));
		result.second.appendChild (GetMemberElement ("auth_response", "string",
				hpassword, document));
		result.second.appendChild (GetMemberElement ("ver", "int",
				"1", document));
		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleValidateReplyFinished ()));
	}

	QPair<QDomElement, QDomElement>LJXmlRPC::GetStartPart (const QString& name, QDomDocument doc)
	{
		QDomElement methodCall = doc.createElement ("methodCall");
		doc.appendChild (methodCall);
		QDomElement methodName = doc.createElement ("methodName");
		methodCall.appendChild (methodName);
		QDomText methodNameText = doc.createTextNode (name);
		methodName.appendChild (methodNameText);
		QDomElement params = doc.createElement ("params");
		methodCall.appendChild (params);
		QDomElement param = doc.createElement ("param");
		params.appendChild (param);
		QDomElement value = doc.createElement ("value");
		param.appendChild (value);
		QDomElement structElem = doc.createElement ("struct");
		value.appendChild (structElem);

		return {methodCall, structElem};
	}

	QDomElement LJXmlRPC::GetMemberElement (const QString& nameVal,
			const QString& typeVal, const QString& value, QDomDocument doc)
	{
		QDomElement member = doc.createElement ("member");
		QDomElement name = doc.createElement ("name");
		member.appendChild (name);
		QDomText nameText = doc.createTextNode (nameVal);
		name.appendChild (nameText);
		QDomElement valueType = doc.createElement ("value");
		member.appendChild (valueType);
		QDomElement type = doc.createElement (typeVal);
		valueType.appendChild (type);
		QDomText text = doc.createTextNode (value);
		type.appendChild (text);

		return member;
	}

	QNetworkRequest LJXmlRPC::CreateNetworkRequest () const
	{
		QNetworkRequest request;
		QByteArray userAgent;
		QTextStream in (&userAgent);
		in << "LeechCraft Blogique " +
				Core::Instance ().GetCoreProxy ()->GetVersion ();
		request.setUrl (QUrl ("http://www.livejournal.com/interface/xmlrpc"));
		request.setRawHeader ("User-Agent", userAgent);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "text/xml");

		return request;
	}

	void LJXmlRPC::handleChallengeReplyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		QXmlQuery query;
		query.setFocus (reply->readAll ());

		QString challenge;
		query.setQuery ("/methodResponse/params/param/value/struct/member[name='challenge']/value/string/text()");
		if (!query.evaluateTo (&challenge))
			return;

		ApiCallQueue_.dequeue () (challenge.simplified ());
		reply->deleteLater ();
	}

	void LJXmlRPC::handleValidateReplyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		QByteArray content = reply->readAll ();
		QDomDocument document (content);
		if (document.elementsByTagName ("fault").isEmpty ())
		{
			emit validatingFinished (true);
			return;
		}
		else
			emit validatingFinished (false);

		QXmlQuery query;
		query.setFocus (content);
		QString errorCode;
		query.setQuery ("/methodResponse/fault/value/struct/member[name='faultCode']/value/int/text()");
		if (!query.evaluateTo (&errorCode))
			return;

		QString errorString;
		query.setQuery ("/methodResponse/fault/value/struct/member[name='faultString']/value/string/text()");
		if (!query.evaluateTo (&errorString))
			return;
		emit error (errorCode.toInt (), errorString);

		reply->deleteLater ();
	}

}
}
}

