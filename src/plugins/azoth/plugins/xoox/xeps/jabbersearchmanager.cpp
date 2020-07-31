/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "jabbersearchmanager.h"
#include <QDomElement>
#include <QXmppClient.h>
#include "util.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsJabberSearch = "jabber:iq:search";

	JabberSearchManager::Item::Item ()
	{
	}

	JabberSearchManager::Item::Item (const QString& jid,
			const QString& first, const QString& last,
			const QString& nick, const QString& email)
	{
		Dictionary_ ["JID"] = jid;
		Dictionary_ [tr ("First name")] = first;
		Dictionary_ [tr ("Last name")] = last;
		Dictionary_ [tr ("Nick")] = nick;
		Dictionary_ [tr ("E-Mail")] = email;
	}

	bool JabberSearchManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq")
			return false;

		const QString& id = elem.attribute ("id");
		if (FieldRequests_.remove (id))
		{
			if (CheckError (elem))
				return true;

			const QDomElement& formElem = elem.firstChildElement ("query");
			if (formElem.isNull ())
				return false;

			emit gotSearchFields (elem.attribute ("from"), QXmppElement (formElem));

			return true;
		}
		else if (SearchRequests_.remove (id))
		{
			if (CheckError (elem))
				return true;

			const QDomElement& items = elem.firstChildElement ("query");
			if (items.isNull ())
				return false;

			const QDomElement& xForm = items.firstChildElement ("x");

			QList<Item> result = !xForm.isNull () ?
					FromForm (xForm) :
					FromStandardItems (items);

			emit gotItems (elem.attribute ("from"), result);

			return true;
		}
		else
			return false;
	}

	void JabberSearchManager::RequestSearchFields (const QString& server)
	{
		QXmppIq iq (QXmppIq::Get);
		iq.setTo (server);

		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsJabberSearch);

		iq.setExtensions (QXmppElementList () << query);

		FieldRequests_ << iq.id ();

		client ()->sendPacket (iq);
	}

	void JabberSearchManager::SubmitSearchRequest (const QString& server, QXmppElement elem)
	{
		QXmppIq iq (QXmppIq::Set);
		iq.setTo (server);
		iq.setExtensions (QXmppElementList () << elem);

		SearchRequests_ << iq.id ();

		client ()->sendPacket (iq);
	}

	void JabberSearchManager::SubmitSearchRequest (const QString& server, const QList<QXmppElement>& fields)
	{
		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsJabberSearch);

		for (const auto& field : fields)
			query.appendChild (field);

		SubmitSearchRequest (server, query);
	}

	void JabberSearchManager::SubmitSearchRequest (const QString& server, const QXmppDataForm& form)
	{
		QXmppElement query;
		query.setTagName ("query");
		query.setAttribute ("xmlns", NsJabberSearch);
		query.appendChild (XooxUtil::Form2XmppElem (form));
		SubmitSearchRequest (server, query);
	}

	bool JabberSearchManager::CheckError (const QDomElement& elem)
	{
		if (elem.firstChildElement ("error").isNull ())
			return false;

		QXmppIq iq;
		iq.parse (elem);

		emit gotServerError (iq);

		return true;
	}

	QList<JabberSearchManager::Item> JabberSearchManager::FromForm (const QDomElement& xForm)
	{
		QList<Item> result;

		QMap<QString, QString> key2label;

		const QDomElement& reported = xForm.firstChildElement ("reported");
		QDomElement field = reported.firstChildElement ("field");
		while (!field.isNull ())
		{
			key2label [field.attribute ("var")] = field.attribute ("label");
			field = field.nextSiblingElement ("field");
		}

		QDomElement fItem = xForm.firstChildElement ("item");
		while (!fItem.isNull ())
		{
			Item item;

			field = fItem.firstChildElement ("field");
			while (!field.isNull ())
			{
				const QString& label = key2label [field.attribute ("var")];
				const QString& value = field.firstChildElement ("value").text ();

				item.Dictionary_ [label] = value;

				field = field.nextSiblingElement ("field");
			}

			if (!item.Dictionary_.isEmpty ())
				result << item;

			fItem = fItem.nextSiblingElement ("item");
		}

		return result;
	}

	QList<JabberSearchManager::Item> JabberSearchManager::FromStandardItems (const QDomElement& items)
	{
		QList<Item> result;

		QDomElement item = items.firstChildElement ("item");
		while (!item.isNull ())
		{
			result << Item (item.attribute ("jid"),
					item.firstChildElement ("first").text (),
					item.firstChildElement ("last").text (),
					item.firstChildElement ("nick").text (),
					item.firstChildElement ("email").text ());

			item = item.nextSiblingElement ("item");
		}

		return result;
	}
}
}
}
