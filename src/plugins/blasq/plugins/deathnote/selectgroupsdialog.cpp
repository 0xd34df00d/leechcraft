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
#include <interfaces/core/ientitymanager.h>
#include <interfaces/blasq/iservice.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>
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

	void SelectGroupsDialog::RequestFriendsGroups ()
	{
		GenerateChallenge ();
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

		QByteArray CreateDomDocumentFromReply (QNetworkReply *reply, QDomDocument &document)
		{
			if (!reply)
				return QByteArray ();

			const auto& content = reply->readAll ();
			reply->deleteLater ();
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
				return QByteArray ();
			}

			return content;
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
	}

	void SelectGroupsDialog::FriendsGroupsRequest (const QString& challenge)
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
		structElem.appendChild (GetSimpleMemberElement ("auth_method", "string",
				"challenge", doc));
		structElem.appendChild (GetSimpleMemberElement ("auth_challenge", "string",
				challenge, doc));
		structElem.appendChild (GetSimpleMemberElement ("username", "string",
				Login_, doc));

		const auto& password = GetAccountPassword (Account_->GetID (), Account_->GetName (), Account_->GetProxy ());
		structElem.appendChild (GetSimpleMemberElement ("auth_response", "string",
				GetHashedChallenge (password, challenge), doc));

		structElem.appendChild (GetSimpleMemberElement ("ver", "int",
				"1", doc));

		QNetworkReply *reply = Account_->GetProxy ()->
				GetNetworkAccessManager ()->post (CreateNetworkRequest (),
						doc.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestFriendsGroupsFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void SelectGroupsDialog::GenerateChallenge ()
	{
		QDomDocument document ("GenerateChallenge");
		QDomElement methodCall = document.createElement ("methodCall");
		document.appendChild (methodCall);

		QDomElement methodName = document.createElement ("methodName");
		methodCall.appendChild (methodName);
		QDomText methodNameText = document.createTextNode ("LJ.XMLRPC.getchallenge");
		methodName.appendChild (methodNameText);

		QNetworkReply *reply = Account_->GetProxy ()->GetNetworkAccessManager ()->
				post (CreateNetworkRequest (), document.toByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleChallengeReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	namespace
	{
		std::optional<QString> GetChallenge (const QDomDocument& doc)
		{
			const auto& replyStruct = doc.documentElement ()
					.firstChildElement ("methodResponse"_qs)
					.firstChildElement ("params"_qs)
					.firstChildElement ("param"_qs)
					.firstChildElement ("value"_qs)
					.firstChildElement ("struct"_qs);
			for (const auto& member : Util::DomChildren (replyStruct, "member"_qs))
				if (member.firstChildElement ("name"_qs).text () == "challenge")
					return member
							.firstChildElement ("value"_qs)
							.firstChildElement ("string"_qs)
							.text ();

			return {};
		}
	}

	void SelectGroupsDialog::handleChallengeReplyFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		const auto& maybeChallenge = GetChallenge (document);
		if (!maybeChallenge)
		{
			qWarning () << "unable to determine challenge from" << content;
			return;
		}

		FriendsGroupsRequest (maybeChallenge->simplified ());
	}

	void SelectGroupsDialog::handleNetworkError (QNetworkReply::NetworkError error)
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();
		qWarning () << Q_FUNC_INFO
				<< "error code:"
				<< error
				<< "error text:"
				<< reply->errorString ();

		Account_->GetProxy ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Blasq",
				reply->errorString (),
				Priority::Warning));
	}

	void SelectGroupsDialog::handleRequestFriendsGroupsFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		QList<FriendsGroup> groups;
		const auto& firstStructElement = document.elementsByTagName ("struct");
		if (firstStructElement.at (0).isNull ())
			return;

		const auto& members = firstStructElement.at (0).childNodes ();
		for (int i = 0, count = members.count (); i < count; ++i)
		{
			const QDomNode& member = members.at (i);
			if (!member.isElement () ||
					member.toElement ().tagName () != "member")
				continue;

			auto res = ParseMember (member);
			if (res.Name_ != "friendgroups")
				continue;

			for (const auto& element : res.Value_)
				groups << CreateGroup (element.toList ());
		}

		for (const auto& group : groups)
		{
			QStandardItem *item = new QStandardItem (group.Name_);
			item->setData (group.Id_);
			item->setEditable (false);
			Model_->appendRow (item);
		}
	}

}
}
}
