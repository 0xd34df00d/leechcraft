/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "offlinedatasource.h"
#include <QXmlStreamWriter>
#include <QDomElement>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>
#include <interfaces/azoth/iproxyobject.h>
#include "vcardstorage.h"
#include "util.h"
#include "glooxaccount.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	void Save (OfflineDataSource_ptr ods, QXmlStreamWriter *w, IProxyObject *proxy)
	{
		w->writeStartElement ("entry");
			w->writeTextElement ("idstr", ods->ID_);
			w->writeTextElement ("name", ods->Name_);
			w->writeTextElement ("authstatus", proxy->AuthStatusToString (ods->AuthStatus_));

			w->writeStartElement ("groups");
				for (const auto& group : ods->Groups_)
					w->writeTextElement ("group", group);
			w->writeEndElement ();
		w->writeEndElement ();
	}

	namespace
	{
		void LoadVCard (const QDomElement& vcardElem, const QString& entryId, GlooxAccount *acc, VCardStorage *storage)
		{
			if (vcardElem.isNull ())
				return;

			storage->SetVCard (XooxUtil::GetBareJID (entryId, acc),
					QByteArray::fromBase64 (vcardElem.text ().toLatin1 ()));
		}

		QString LoadEntryID (const QDomElement& entry)
		{
			const auto& idStrElem = entry.firstChildElement ("idstr");
			if (!idStrElem.isNull ())
				return idStrElem.text ();

			const auto& idElem = entry.firstChildElement ("id");
			return QString::fromUtf8 (QByteArray::fromPercentEncoding (idElem.text ().toLatin1 ()));
		}
	}

	void Load (OfflineDataSource_ptr ods,
			const QDomElement& entry,
			IProxyObject *proxy,
			GlooxAccount *acc)
	{
		const auto& entryID = LoadEntryID (entry);

		auto groups = Util::Map (Util::DomChildren (entry.firstChildElement ("groups"), "group"),
				[] (const QDomElement& group) { return group.text (); });
		groups.removeAll ({});

		ods->Name_ = entry.firstChildElement ("name").text ();
		ods->ID_ = entryID;
		ods->Groups_ = groups;

		const auto& authStatusText = entry.firstChildElement ("authstatus").text ();
		ods->AuthStatus_ = proxy->AuthStatusFromString (authStatusText);

		LoadVCard (entry.firstChildElement ("vcard"), entryID,
				acc, acc->GetParentProtocol ()->GetVCardStorage ());
	}
}
}
}
