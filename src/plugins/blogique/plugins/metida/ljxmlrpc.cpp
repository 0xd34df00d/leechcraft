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
#include "ljfriendentry.h"

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

	void LJXmlRPC::AddNewFriend (const QString& username,
			const QString& bgcolor, const QString& fgcolor, uint groupId)
	{
		ApiCallQueue_ << [username, bgcolor, fgcolor, groupId, this] (const QString& challenge)
				{ AddNewFriendRequest (username, bgcolor, fgcolor, groupId, challenge); };
		GenerateChallenge ();
	}

	void LJXmlRPC::DeleteFriend (const QString& username)
	{
		ApiCallQueue_ << [username, this] (const QString& challenge)
				{ DeleteFriendRequest (username, challenge); };
		GenerateChallenge ();
	}

	void LJXmlRPC::AddGroup (const QString& name, bool isPublic, int id)
	{
		ApiCallQueue_ << [name, isPublic, id, this] (const QString& challenge)
				{ AddGroupRequest (name, isPublic, id, challenge); };
		GenerateChallenge ();
	}

	void LJXmlRPC::DeleteGroup (int id)
	{
		ApiCallQueue_ << [id, this] (const QString& challenge)
				{ DeleteGroupRequest (id, challenge); };
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

		QDomElement GetSimpleMemberElement (const QString& nameVal,
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

		QPair<QDomElement, QDomElement> GetComplexMemberElement (const QString& nameVal,
				const QString& typeVal, QDomDocument doc)
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
			QDomElement dataField;
			if (typeVal == "array")
			{
				dataField = doc.createElement ("data");
				type.appendChild (dataField);
			}
			else if (typeVal == "struct")
				dataField = type;

			return { member, dataField };
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
		result.second.appendChild (GetSimpleMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetSimpleMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetSimpleMemberElement ("username", "string",
				login, document));
		result.second.appendChild (GetSimpleMemberElement ("auth_response", "string",
				GetPassword (password, challenge), document));
		result.second.appendChild (GetSimpleMemberElement ("ver", "int",
				"1", document));
		//TODO
		result.second.appendChild (GetSimpleMemberElement ("clientversion", "string",
				Util::SysInfo::GetOSName () +
						"-LeechCraft Blogique: " +
						Core::Instance ().GetCoreProxy ()->GetVersion (),
				document));
		result.second.appendChild (GetSimpleMemberElement ("getmoods", "int",
				"0", document));
		result.second.appendChild (GetSimpleMemberElement ("getmenus", "int",
				"0", document));
		result.second.appendChild (GetSimpleMemberElement ("getpickws", "int",
				"1", document));
		result.second.appendChild (GetSimpleMemberElement ("getpickwurls", "int",
				"1", document));
		result.second.appendChild (GetSimpleMemberElement ("getcaps", "int",
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

		result.second.appendChild (GetSimpleMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetSimpleMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetSimpleMemberElement ("username", "string",
				login, document));
		result.second.appendChild (GetSimpleMemberElement ("auth_response", "string",
				GetPassword (password, challenge), document));
		result.second.appendChild (GetSimpleMemberElement ("ver", "int",
				"1", document));
		result.second.appendChild (GetSimpleMemberElement ("includebdays", "boolean",
				"1", document));
		result.second.appendChild (GetSimpleMemberElement ("includefriendof", "boolean",
				"1", document));
		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestFriendsInfoFinished ()));
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
			return;

		const auto& members = firstStructElement.at (0).childNodes ();
		QHash<QString, LJFriendEntry_ptr> frHash;
		for (int i = 0, count = members.count (); i < count; ++i)
		{
			const QDomNode& member = members.at (i);
			if (!member.isElement () ||
				member.toElement ().tagName () != "member")
				continue;

			auto res = ParseMember (member);
			if (res.Name () == "friends" ||
					res.Name () == "added" ||
					res.Name () == "friendofs")
			{
				for (const auto& moodEntry : res.Value ())
				{
					LJFriendEntry_ptr fr = std::make_shared<LJFriendEntry> ();
					bool isCommunity = false, personal = false;
					for (const auto& field : moodEntry.toList ())
					{
						LJParserTypes::LJParseProfileEntry fieldEntry =
								field.value<LJParserTypes::LJParseProfileEntry> ();
						if (fieldEntry.Name () == "defaultpicurl")
							fr->SetAvatarUrl (fieldEntry.ValueToUrl ());
						else if (fieldEntry.Name () == "bgcolor")
							fr->SetBGColor (fieldEntry.ValueToString ());
						else if (fieldEntry.Name () == "fgcolor")
							fr->SetFGColor (fieldEntry.ValueToString ());
						else if (fieldEntry.Name () == "groupmask")
							fr->SetGroupMask (fieldEntry.ValueToInt ());
						else if (fieldEntry.Name () == "fullname")
							fr->SetFullName (fieldEntry.ValueToString ());
						else if (fieldEntry.Name () == "username")
							fr->SetUserName (fieldEntry.ValueToString ());
						else if (fieldEntry.Name () == "type")
							isCommunity = (fieldEntry.ValueToString () == "community");
						else if (fieldEntry.Name () == "journaltype")
						{
							isCommunity = (fieldEntry.ValueToString () == "C");
							personal = (fieldEntry.ValueToString () == "P");
						}
						else if (fieldEntry.Name () == "birthday")
							fr->SetBirthday (fieldEntry.ValueToString ());

						if (res.Name () == "friends" ||
								res.Name () == "added")
							fr->SetMyFriend (true);

						if (res.Name () == "friendofs")
							fr->SetFriendOf (true);
					}

					if (!isCommunity ||
							personal)
					{
						if (res.Name () == "friendofs" &&
								frHash.contains (fr->GetUserName ()))
							frHash [fr->GetUserName ()]->SetFriendOf (true);
						else if ((res.Name () == "friends" ||
								res.Name () == "added") &&
								frHash.contains (fr->GetUserName ()))
							frHash [fr->GetUserName ()]->SetMyFriend (true);
						else
							frHash [fr->GetUserName ()] = fr;
					}
				}
				Account_->AddFriends (frHash.values ());
			}
		}
	}

	void LJXmlRPC::AddNewFriendRequest (const QString& username,
			const QString& bgcolor, const QString& fgcolor,
			int groupId, const QString& challenge)
	{
		QDomDocument document ("AddNewFriendRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editfriends", document);
		document.appendChild (result.first);
		result.second.appendChild (GetSimpleMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetSimpleMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetSimpleMemberElement ("username", "string",
				Account_->GetOurLogin (), document));
		result.second.appendChild (GetSimpleMemberElement ("auth_response", "string",
				GetPassword (Account_->GetPassword (), challenge), document));
		result.second.appendChild (GetSimpleMemberElement ("ver", "int",
				"1", document));

		auto array = GetComplexMemberElement ("add", "array", document);
 		result.second.appendChild (array.first);
		auto structField = document.createElement ("struct");
		array.second.appendChild (structField);

		structField.appendChild (GetSimpleMemberElement ("username", "string",
				username, document));
		if (!fgcolor.isEmpty ())
			structField.appendChild (GetSimpleMemberElement ("fgcolor", "string",
					fgcolor, document));
		if (!bgcolor.isEmpty ())
			structField.appendChild (GetSimpleMemberElement ("bgcolor", "string",
					bgcolor, document));
		structField.appendChild (GetSimpleMemberElement ("groupmask", "int",
				QString::number (groupId), document));

		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAddNewFriendReplyFinished ()));
	}

	void LJXmlRPC::DeleteFriendRequest (const QString& username, const QString& challenge)
	{
		QDomDocument document ("DeleteFriendRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editfriends", document);
		document.appendChild (result.first);
		result.second.appendChild (GetSimpleMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetSimpleMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetSimpleMemberElement ("username", "string",
				Account_->GetOurLogin (), document));
		result.second.appendChild (GetSimpleMemberElement ("auth_response", "string",
				GetPassword (Account_->GetPassword (), challenge), document));
		result.second.appendChild (GetSimpleMemberElement ("ver", "int",
				"1", document));

		auto array = GetComplexMemberElement ("delete", "array", document);
		result.second.appendChild (array.first);
		QDomElement valueField = document.createElement ("value");
		array.second.appendChild (valueField);
		QDomElement valueType = document.createElement ("string");
		valueField.appendChild (valueType);
		QDomText text = document.createTextNode (username);
		valueType.appendChild (text);

		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyWithProfileUpdate ()));
	}

	void LJXmlRPC::AddGroupRequest (const QString& name, bool isPublic, int id,
			const QString& challenge)
	{
		QDomDocument document ("AddNewFriendRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editfriendgroups", document);
		document.appendChild (result.first);
		result.second.appendChild (GetSimpleMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetSimpleMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetSimpleMemberElement ("username", "string",
				Account_->GetOurLogin (), document));
		result.second.appendChild (GetSimpleMemberElement ("auth_response", "string",
				GetPassword (Account_->GetPassword (), challenge), document));
		result.second.appendChild (GetSimpleMemberElement ("ver", "int",
				"1", document));

		auto data = GetComplexMemberElement ("set", "struct", document);
		result.second.appendChild (data.first);
		auto subStruct = GetComplexMemberElement (QString::number (id), "struct", document);
		data.second.appendChild (subStruct.first);
		subStruct.second.appendChild (GetSimpleMemberElement ("name", "string",
				name, document));
		subStruct.second.appendChild (GetSimpleMemberElement ("public", "boolean",
				 isPublic ? "1" : "0", document));

		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyWithProfileUpdate ()));
	}

	void LJXmlRPC::DeleteGroupRequest (int id, const QString& challenge)
	{
		QDomDocument document ("DeleteGroupRequest");
		auto result = GetStartPart ("LJ.XMLRPC.editfriendgroups", document);
		document.appendChild (result.first);
		result.second.appendChild (GetSimpleMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetSimpleMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetSimpleMemberElement ("username", "string",
				Account_->GetOurLogin (), document));
		result.second.appendChild (GetSimpleMemberElement ("auth_response", "string",
				GetPassword (Account_->GetPassword (), challenge), document));
		result.second.appendChild (GetSimpleMemberElement ("ver", "int",
				"1", document));

		auto array = GetComplexMemberElement ("delete", "array", document);
		result.second.appendChild (array.first);
		QDomElement valueField = document.createElement ("value");
		array.second.appendChild (valueField);
		QDomElement valueType = document.createElement ("int");
		valueField.appendChild (valueType);
		QDomText text = document.createTextNode (QString::number (id));
		valueType.appendChild (text);

		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						  document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyWithProfileUpdate ()));
	}

	void LJXmlRPC::PostEventRequest (const LJEvent& event, const QString& challenge)
	{
		QDomDocument document ("PostEventRequest");
		auto result = GetStartPart ("LJ.XMLRPC.postevent", document);
		document.appendChild (result.first);
		result.second.appendChild (GetSimpleMemberElement ("auth_method", "string",
				"challenge", document));
		result.second.appendChild (GetSimpleMemberElement ("auth_challenge", "string",
				challenge, document));
		result.second.appendChild (GetSimpleMemberElement ("username", "string",
				Account_->GetOurLogin (), document));
		result.second.appendChild (GetSimpleMemberElement ("auth_response", "string",
				GetPassword (Account_->GetPassword (), challenge), document));
		result.second.appendChild (GetSimpleMemberElement ("ver", "int",
				"1", document));

		result.second.appendChild (GetSimpleMemberElement ("event", "string",
				event.Event_, document));
		result.second.appendChild (GetSimpleMemberElement ("subject", "string",
				event.Subject_, document));
// 		result.second.appendChild (GetSimpleMemberElement (, document));
// 		result.second.appendChild (GetSimpleMemberElement (, document));
// 		result.second.appendChild (GetSimpleMemberElement (, document));
// 		result.second.appendChild (GetSimpleMemberElement (, document));
// 		result.second.appendChild (GetSimpleMemberElement (, document));

		QNetworkReply *reply = Core::Instance ().GetCoreProxy ()->
		GetNetworkAccessManager ()->post (CreateNetworkRequest (),
										  document.toByteArray ());

		connect (reply,
				 SIGNAL (finished ()),
				 this,
		   SLOT (handleReplyWithProfileUpdate ()));
	}

	void LJXmlRPC::UpdateProfileInfo ()
	{
		Validate (Account_->GetOurLogin (), Account_->GetPassword ());
	}

	void LJXmlRPC::Submit (const LJEvent& event)
	{
		ApiCallQueue_ << [event, this] (const QString& challenge)
				{ PostEventRequest (event, challenge); };
		GenerateChallenge ();
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
								group.RealId_ = (1 << group.Id_) + 1;
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

	void LJXmlRPC::handleRequestFriendsInfoFinished ()
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

	void LJXmlRPC::handleAddNewFriendReplyFinished ()
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

	void LJXmlRPC::handleReplyWithProfileUpdate ()
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
			Account_->updateProfile ();
			return;
		}

		ParseForError (content);
	}

}
}
}
