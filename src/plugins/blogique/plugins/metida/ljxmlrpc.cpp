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
#include <QDomDocument>
#include <QtDebug>
#include <QCryptographicHash>
#include <QDateTime>
#include <QNetworkReply>
#include <QXmlQuery>
#include <util/sysinfo.h>
#include "profiletypes.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJXmlRPC::LJXmlRPC (LJAccount *acc, QObject *parent)
	: QObject (parent)
	, Account_ (acc)
	{
	}

	void LJXmlRPC::Validate (const QString& login, const QString& password)
	{
		ApiCallQueue_ << [login, password, this] (const QString& challenge)
				{ ValidateAccountData (login, password, challenge); };
		ApiCallQueue_ << [login, password, this] (const QString& challenge)
				{ RequestFriendsInfo (login, password, challenge); };
		//TODO get communities info via parsing page
		GenerateChallenge ();
	}

	namespace
	{
		QPair<QDomElement, QDomElement> GetStartPart (const QString& name,
				QDomDocument doc)
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

		QDomElement GetMemberElement (const QString& nameVal,
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

		QNetworkRequest CreateNetworkRequest ()
		{
			QNetworkRequest request;
			auto userAgent = "LeechCraft Blogique " +
					Core::Instance ().GetCoreProxy ()->GetVersion ().toUtf8 ();
			request.setUrl (QUrl ("http://www.livejournal.com/interface/xmlrpc"));
			request.setRawHeader ("User-Agent", userAgent);
			request.setHeader (QNetworkRequest::ContentTypeHeader, "text/xml");

			return request;
		}
	}

	void LJXmlRPC::GenerateChallenge () const
	{
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

	namespace
	{
		QString GetPassword (const QString& password, const QString& challenge)
		{
			const QByteArray passwordHash = QCryptographicHash::hash (password.toUtf8 (),
					QCryptographicHash::Md5).toHex ();
			return QCryptographicHash::hash ((challenge + passwordHash).toUtf8 (),
				QCryptographicHash::Md5).toHex ();
		}
	}

	void LJXmlRPC::ValidateAccountData (const QString& login,
			const QString& password, const QString& challenge)
	{
		QDomDocument document ("ValidateRequest");
		auto result = GetStartPart ("LJ.XMLRPC.login", document);
		document.appendChild (result.first);
		result.second.appendChild (GetMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetMemberElement ("username", "string",
				login, document));
		result.second.appendChild (GetMemberElement ("auth_response", "string",
				GetPassword (password, challenge), document));
		result.second.appendChild (GetMemberElement ("ver", "int",
				"1", document));
		//TODO
// 		result.second.appendChild (GetMemberElement ("clientversion", "string",
// 				Util::SysInfo::GetOSName () +
// 						"-LeechCraft Blogique " +
// 						Core::Instance ().GetCoreProxy ()->GetVersion (),
// 				document));
		result.second.appendChild (GetMemberElement ("getmoods", "int",
				"0", document));
		result.second.appendChild (GetMemberElement ("getmenus", "int",
				"0", document));
		result.second.appendChild (GetMemberElement ("getpickws", "int",
				"1", document));
		result.second.appendChild (GetMemberElement ("getpickwurls", "int",
				"1", document));
		result.second.appendChild (GetMemberElement ("getcaps", "int",
				"1", document));

		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleValidateReplyFinished ()));
	}

	void LJXmlRPC::RequestFriendsInfo (const QString& login,
			const QString& password, const QString& challenge)
	{
		QDomDocument document ("GetFriendsInfo");
		auto result = GetStartPart ("LJ.XMLRPC.getfriends", document);
		document.appendChild (result.first);

		result.second.appendChild (GetMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetMemberElement ("username", "string",
				login, document));
		result.second.appendChild (GetMemberElement ("auth_response", "string",
				GetPassword (password, challenge), document));
		result.second.appendChild (GetMemberElement ("ver", "int",
				"1", document));
		result.second.appendChild (GetMemberElement ("includebdays", "boolean",
				"1", document));

		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestFriendsInfoyFinished ()));
	}

	void LJXmlRPC::ParseForError (const QByteArray& content)
	{
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
	}

	namespace
	{
		QVariantList ParseValue (const QDomNode& node);

		LJParserTypes::LJParseProfileEntry ParseMember (const QDomNode& node)
		{
			const auto& memberFields = node.childNodes ();
			const auto& memberNameField = memberFields.at (0);
			const auto& memberValueField = memberFields.at (1);
			QString memberName;
			QVariantList memberValue;
			if (memberNameField.isElement () &&
					memberNameField.toElement ().tagName () == "name")
				memberName = memberNameField.toElement ().text ();

			if (memberValueField.isElement ())
				memberValue = ParseValue (memberValueField);

			return LJParserTypes::LJParseProfileEntry (memberName, memberValue);
		}

		QVariantList ParseValue (const QDomNode& node)
		{
			QVariantList result;
			const auto& valueNode = node.firstChild ();
			const auto& valueElement = valueNode.toElement ();
			QString type = valueElement.tagName ();
			if (type == "string" ||
					type == "int" ||
					type == "i4" ||
					type == "double" ||
					type == "boolean")
				result << valueElement.text ();
			else if (type == "dateTime.iso8601")
				result << QDateTime::fromString (valueElement.text (), Qt::ISODate);
			else if (type == "base64")
				result << QString::fromUtf8 (QByteArray::fromBase64 (valueElement.text ().toUtf8 ()));
			else if (type == "array")
			{
				const auto& arrayElements = valueNode.firstChild ().childNodes ();
				QVariantList array;
				for (int i = 0, count = arrayElements.count (); i < count; ++i)
					array << QVariant::fromValue<QVariantList> (ParseValue (arrayElements.at (i)));

				result << array;
			}
			else if (type == "struct")
			{
				const auto& structMembers = valueNode.childNodes ();
				for (int i = 0, count = structMembers.count (); i < count; ++i)
					result << QVariant::fromValue<LJParserTypes::LJParseProfileEntry> (ParseMember (structMembers.at (i)));
			}

			return result;
		}
	}

	void LJXmlRPC::ParseFriends (const QDomDocument& document)
	{
		const auto& firstStructElement = document.elementsByTagName ("struct");

		if (firstStructElement.at (0).isNull ())
			return ;

		const auto& members = firstStructElement.at (0).childNodes ();
		for (int i = 0, count = members.count (); i < count; ++i)
		{
			const QDomNode& member = members.at (i);
			if (!member.isElement () ||
				member.toElement ().tagName () != "member")
				continue;

			auto res = ParseMember (member);
		}
	}


	void LJXmlRPC::handleChallengeReplyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		const auto& content = reply->readAll ();
		reply->deleteLater ();
		QDomDocument document;
		QString errorMsg;
		int errorLine = -1, errorColumn = -1;
		if (!document.setContent (content, &errorMsg, &errorLine, &errorColumn))
		{
			qWarning () << Q_FUNC_INFO
					<< errorMsg
					<< "in line:"
					<< errorLine
					<< "column:"
					<< errorColumn;
			return;
		}

		QXmlQuery query;
		query.setFocus (content);

		QString challenge;
		query.setQuery ("/methodResponse/params/param/value/struct/member[name='challenge']/value/string/text()");
		if (!query.evaluateTo (&challenge))
			return;

		ApiCallQueue_.dequeue () (challenge.simplified ());
		if (!ApiCallQueue_.isEmpty ())
			GenerateChallenge ();
	}

	namespace
	{
		struct Id2ProfileField
		{
			QHash<QString, std::function<void (LJProfileData&,
					const LJParserTypes::LJParseProfileEntry&)>> Id2ProfileField_;

			Id2ProfileField ()
			{
				Id2ProfileField_ ["defaultpicurl"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					profile.AvatarUrl_ = entry.ValueToUrl ();
				};
				Id2ProfileField_ ["friendgroups"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					for (const auto& friendGroupEntry : entry.Value ())
					{
						LJFriendGroup group;
						for (const auto& field : friendGroupEntry.toList ())
						{
							LJParserTypes::LJParseProfileEntry fieldEntry =
							field.value<LJParserTypes::LJParseProfileEntry> ();
							if (fieldEntry.Name () == "public")
								group.Public_ = fieldEntry.ValueToBool ();
							else if (fieldEntry.Name () == "name")
								group.Name_ = fieldEntry.ValueToString ();
							else if (fieldEntry.Name () == "id")
							{
								group.Id_ = fieldEntry.ValueToInt ();
								group.RealId_ = (group.Id_ << 8) + 1;
							}
							else if (fieldEntry.Name () == "sortorder")
								group.SortOrder_ = fieldEntry.ValueToInt ();
						}
						profile.FriendGroups_ << group;
					}
				};
				Id2ProfileField_ ["usejournals"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					for (const auto& val : entry.Value ())
						profile.Communities_ << val.toList ().value (0).toString ();
				};
				Id2ProfileField_ ["fullname"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					profile.FullName_ = entry.ValueToString ();
				};
				Id2ProfileField_ ["moods"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					for (const auto& moodEntry : entry.Value ())
					{
						LJMood mood;
						for (const auto& field : moodEntry.toList ())
						{
							LJParserTypes::LJParseProfileEntry fieldEntry =
							field.value<LJParserTypes::LJParseProfileEntry> ();
							if (fieldEntry.Name () == "parent")
								mood.Parent_ = fieldEntry.ValueToLongLong ();
							else if (fieldEntry.Name () == "name")
								mood.Name_ = fieldEntry.ValueToString ();
							else if (fieldEntry.Name () == "id")
								mood.Id_ = fieldEntry.ValueToLongLong ();
						}
						profile.Moods_ << mood;
					}
				};
				Id2ProfileField_ ["userid"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					profile.UserId_ = entry.ValueToLongLong ();
				};
				Id2ProfileField_ ["caps"] = [] (LJProfileData& profile,
						const LJParserTypes::LJParseProfileEntry& entry)
				{
					profile.Caps_ = entry.ValueToLongLong ();
				};
			}
		};

		LJProfileData ParseProfileInfo (const QDomDocument& document)
		{
			static Id2ProfileField id2field;
			const auto& firstStructElement = document.elementsByTagName ("struct");
			LJProfileData profile;
			if (firstStructElement.at (0).isNull ())
				return LJProfileData ();

			const auto& members = firstStructElement.at (0).childNodes ();
			for (int i = 0, count = members.count (); i < count; ++i)
			{
				const QDomNode& member = members.at (i);
				if (!member.isElement () ||
						member.toElement ().tagName () != "member")
					continue;

				auto res = ParseMember (member);
				if (id2field.Id2ProfileField_.contains (res.Name ()))
					id2field.Id2ProfileField_ [res.Name ()] (profile, res);
			}

			return profile;
		}
	}

	void LJXmlRPC::handleValidateReplyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		const auto& content = reply->readAll ();
		reply->deleteLater ();
		QDomDocument document;
		QString errorMsg;
		int errorLine = -1, errorColumn = -1;
		if (!document.setContent (content, &errorMsg, &errorLine, &errorColumn))
		{
			qWarning () << Q_FUNC_INFO
					<< errorMsg
					<< "in line:"
					<< errorLine
					<< "column:"
					<< errorColumn;
			return;
		}

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			const LJProfileData& data = ParseProfileInfo (document);
			emit profileUpdated (data);
			emit validatingFinished (true);
			return;
		}
		else
			emit validatingFinished (false);

		ParseForError (content);
	}

	void LJXmlRPC::handleRequestFriendsInfoyFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		const auto& content = reply->readAll ();
		reply->deleteLater ();
		QDomDocument document;
		QString errorMsg;
		int errorLine = -1, errorColumn = -1;
		if (!document.setContent (content, &errorMsg, &errorLine, &errorColumn))
		{
			qWarning () << Q_FUNC_INFO
					<< errorMsg
					<< "in line:"
					<< errorLine
					<< "column:"
					<< errorColumn;
			return;
		}

		if (document.elementsByTagName ("fault").isEmpty ())
		{
			ParseFriends (document);
			return;
		}

		ParseForError (content);
	}

}
}
}

