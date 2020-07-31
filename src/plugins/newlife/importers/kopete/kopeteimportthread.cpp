/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "kopeteimportthread.h"
#include <QFile>
#include <QDomDocument>
#include <QDateTime>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/structures.h>

namespace LC
{
namespace NewLife
{
namespace Importers
{
	KopeteImportThread::KopeteImportThread (const ICoreProxy_ptr& proxy, const QString& proto, const QStringList& files)
	: Proxy_ (proxy)
	, Proto_ (proto)
	, Files_ (files)
	{
		connect (this,
				SIGNAL (finished ()),
				this,
				SLOT (deleteLater ()));
	}

	void KopeteImportThread::run ()
	{
		msleep (300);

		for (const auto& file : Files_)
		{
			ParseFile (file);
			msleep (100);
		}
	}

	namespace
	{
		void ParseHead (QDomElement root, int& month, int& year, QString& contact, QString& ourContact)
		{
			const auto& head = root.firstChildElement ("head");

			auto date = head.firstChildElement ("date");
			month = date.attribute ("month").toInt ();
			year = date.attribute ("year").toInt ();

			auto contactElem = head.firstChildElement ("contact");
			while (!contactElem.isNull ())
			{
				if (contactElem.attribute ("type") != "myself")
					contact = contactElem.attribute ("contactId");
				else
					ourContact = contactElem.attribute ("contactId");

				contactElem = contactElem.nextSiblingElement ("contact");
			}
		}
	}

	void KopeteImportThread::ParseFile (const QString& filename)
	{
		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		QDomDocument doc;
		if (!doc.setContent (&file))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing file"
					<< file.fileName ();
			return;
		}

		int month = 0, year = 0;
		QString contact;
		QString ourContact;
		ParseHead (doc.documentElement (), month, year, contact, ourContact);
		if (contact.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "got empty contact for"
					<< file.fileName ();
			return;
		}

		QVariantList list;

		auto msg = doc.documentElement ().firstChildElement ("msg");
		while (!msg.isNull ())
		{
			const auto& rawTime = msg.attribute ("time").split (' ', Qt::SkipEmptyParts);

			QVariantMap result;
			result ["EntryID"] = contact;
			result ["DateTime"] = QDateTime (QDate (year, month, rawTime.value (0).toInt ()),
						QTime::fromString (rawTime.value (1), "h:m:s"));
			result ["Direction"] = msg.attribute ("in") == "0" ? "in" : "out";
			result ["Body"] = msg.text ();
			result ["MessageType"] = "chat";

			list << result;

			msg = msg.nextSiblingElement ("msg");
		}

		if (list.isEmpty ())
			return;

		Entity e;
		e.Additional_ ["History"] = list;
		e.Mime_ = "x-leechcraft/im-history-import";
		e.Additional_ ["AccountName"] = ourContact;
		e.Additional_ ["AccountID"] = ourContact;
		e.Parameters_ = OnlyHandle | FromUserInitiated;
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
}
