/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "selectgroupsdialog.h"
#include <QDomDocument>
#include <QNetworkRequest>
#include <QStandardItemModel>
#include <QtDebug>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/blasq/iservice.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>
#include <util/svcauth/ljutils.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include "fotobilderaccount.h"
#include "util.h"

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	SelectGroupsDialog::SelectGroupsDialog (const QString& login, FotoBilderAccount *acc,
			QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	, Login_ (login)
	, Account_ (acc)
	{
		Ui_.setupUi (this);

		Ui_.Groups_->setModel (Model_);
		Model_->setHorizontalHeaderLabels ({ tr ("Groups") });

		RequestFriendsGroups ();
	}

	uint SelectGroupsDialog::GetSelectedGroupId () const
	{
		return Ui_.Groups_->selectionModel ()->selectedRows ()
				.value (0).data (Qt::UserRole + 1).toUInt ();
	}

	namespace
	{
		void NotifyError (const QString& msg)
		{
			const auto entity = Util::MakeNotification ("Blasq", msg, Priority::Warning);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (entity);
		}

		using ParseFriendsGroupsResult = Util::Either<QString, QList<FriendsGroup>>;
		ParseFriendsGroupsResult ParseFriendsGroupsResponse (const QByteArray& response);
	}

	Util::ContextTask<> SelectGroupsDialog::RequestFriendsGroups ()
	{
		co_await Util::AddContextObject { *this };

		const auto& challengeResponse = co_await Util::LJ::RequestChallenge ({
				.NAM_ = *GetProxyHolder ()->GetNetworkAccessManager (),
				.UserAgent_ = "LeechCraft Blasq " + GetProxyHolder ()->GetVersion ().toUtf8 (),
			});
		const auto& challenge = co_await Util::WithHandler (challengeResponse,
				[&] (const auto& error) { NotifyError (tr ("Unable to get challenge.") + ' ' + error.Text_); });

		const auto& request = GetFriendsGroupsRequestBody (challenge);

		const auto reply = GetProxyHolder ()->GetNetworkAccessManager ()->post (CreateNetworkRequest (), request);
		const auto response = co_await *reply;
		const auto responseBody = co_await Util::WithHandler (response.ToEither ("Request friends groups"), NotifyError);
		const auto groups = co_await Util::WithHandler (ParseFriendsGroupsResponse (responseBody), NotifyError);
		for (const auto& group : groups)
		{
			const auto item = new QStandardItem { group.Name_ };
			item->setData (group.Id_);
			item->setEditable (false);
			Model_->appendRow (item);
		}
	}

	QNetworkRequest SelectGroupsDialog::CreateNetworkRequest ()
	{
		QNetworkRequest request;
		auto userAgent = "LeechCraft Blasq " + Account_->GetProxy ()->GetVersion ().toUtf8 ();
		request.setUrl (QUrl ("http://www.livejournal.com/interface/xmlrpc"));
		request.setRawHeader ("User-Agent", userAgent);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "text/xml");

		return request;
	}

	namespace
	{
		FriendsGroup CreateGroup (const QVariantList& data)
		{
			FriendsGroup group;
			for (const auto& field : data)
			{
				ParsedMember fieldEntry = field.value<ParsedMember> ();
				if (fieldEntry.Name_ == "public")
					group.Public_ = fieldEntry.Value_.value (0).toInt ();
				else if (fieldEntry.Name_ == "name")
					group.Name_ = fieldEntry.Value_.value (0).toString ();
				else if (fieldEntry.Name_ == "id")
				{
					group.Id_ = fieldEntry.Value_.value (0).toInt ();
					group.RealId_ = (1 << group.Id_) + 1;
				}
				else if (fieldEntry.Name_ == "sortorder")
					group.SortOrder_ = fieldEntry.Value_.value (0).toInt ();
			}

			return group;
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

		QVariantList ParseValue (const QDomNode& node);

		ParsedMember ParseMember (const QDomNode& node)
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

			return { memberName, memberValue };
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
					result << QVariant::fromValue<ParsedMember> (ParseMember (structMembers.at (i)));
			}

			return result;
		}

		ParseFriendsGroupsResult ParseFriendsGroupsResponse (const QByteArray& response)
		{
			QDomDocument document;
			if (!document.setContent (response))
			{
				qWarning () << "unable to parse" << response;
				return { Util::AsLeft, SelectGroupsDialog::tr ("Unable to parse network response.") };
			}

			const auto& firstStructElement = document.elementsByTagName ("struct");
			if (!firstStructElement.at (0).isElement ())
			{
				qWarning () << "no groups" << response;
				return { Util::AsLeft, SelectGroupsDialog::tr ("Empty friends groups response.") };
			}

			QList<FriendsGroup> groups;
			for (const auto& member : Util::DomChildren (firstStructElement.at (0).toElement (), "member"_qs))
			{
				auto res = ParseMember (member);
				if (res.Name_ != "friendgroups")
					continue;

				for (const auto& element : res.Value_)
					groups << CreateGroup (element.toList ());
			}

			return groups;
		}
	}

	QByteArray SelectGroupsDialog::GetFriendsGroupsRequestBody (const QString& challenge)
	{
		QDomDocument doc ("GetFriendsGroups");
		QDomElement methodCall = doc.createElement ("methodCall");
		doc.appendChild (methodCall);
		QDomElement methodName = doc.createElement ("methodName");
		methodCall.appendChild (methodName);
		QDomText methodNameText = doc.createTextNode ("LJ.XMLRPC.getfriendgroups");
		methodName.appendChild (methodNameText);
		QDomElement params = doc.createElement ("params");
		methodCall.appendChild (params);
		QDomElement param = doc.createElement ("param");
		params.appendChild (param);
		QDomElement value = doc.createElement ("value");
		param.appendChild (value);
		QDomElement structElem = doc.createElement ("struct");
		value.appendChild (structElem);
		structElem.appendChild (GetSimpleMemberElement ("auth_method", "string", "challenge", doc));
		structElem.appendChild (GetSimpleMemberElement ("auth_challenge", "string", challenge, doc));
		structElem.appendChild (GetSimpleMemberElement ("username", "string", Login_, doc));

		const auto& password = GetAccountPassword (Account_->GetID (), Account_->GetName (), GetProxyHolder ());
		structElem.appendChild (GetSimpleMemberElement ("auth_response", "string", GetHashedChallenge (password, challenge), doc));

		structElem.appendChild (GetSimpleMemberElement ("ver", "int", "1", doc));

		return doc.toByteArray ();
	}
}
}
}
