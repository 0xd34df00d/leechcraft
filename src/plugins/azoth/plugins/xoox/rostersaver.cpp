/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rostersaver.h"
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/sll/domchildrenrange.h>
#include <interfaces/azoth/iproxyobject.h>
#include "glooxprotocol.h"
#include "glooxaccount.h"
#include "glooxclentry.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	RosterSaver::RosterSaver (GlooxProtocol *proto, IProxyObject *proxy, QObject *parent)
	: QObject { parent }
	, Proto_ { proto }
	, Proxy_ { proxy }
	{
		LoadRoster ();

		for (const auto account : Proto_->GetRegisteredAccounts ())
			handleAccount (account);

		connect (Proto_,
				SIGNAL (accountAdded (QObject*)),
				this,
				SLOT (handleAccount (QObject*)));
	}

	void RosterSaver::LoadRoster ()
	{
		QFile rosterFile { Util::CreateIfNotExists ("azoth/xoox").absoluteFilePath ("roster.xml") };
		if (!rosterFile.exists ())
			return;
		if (!rosterFile.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open roster file"
					<< rosterFile.fileName ()
					<< rosterFile.errorString ();
			return;
		}

		QDomDocument doc;
		QString errStr;
		int errLine = 0;
		int errCol = 0;
		if (!doc.setContent (&rosterFile, &errStr, &errLine, &errCol))
		{
			qWarning () << Q_FUNC_INFO
					<< errStr
					<< errLine
					<< ":"
					<< errCol;
			return;
		}

		QDomElement root = doc.firstChildElement ("roster");
		if (root.attribute ("formatversion") != "1")
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown format version"
					<< root.attribute ("formatversion");
			return;
		}

		QMap<QByteArray, GlooxAccount*> id2account;
		for (const auto accObj : Proto_->GetRegisteredAccounts ())
		{
			const auto acc = qobject_cast<GlooxAccount*> (accObj);
			id2account [acc->GetAccountID ()] = acc;
		}

		for (const auto& account : Util::DomChildren (root, "account"))
		{
			const auto& id = account.firstChildElement ("id").text ().toUtf8 ();
			if (!id2account.contains (id))
				continue;

			const auto acc = id2account [id];

			// TODO remove this some time early 2018.
			// To allow some transition time for duplicates removal.
			QSet<QString> existingEntries;

			for (const auto& entry : Util::DomChildren (account.firstChildElement ("entries"), "entry"))
			{
				const auto ods = std::make_shared<OfflineDataSource> ();
				Load (ods, entry, Proxy_, acc);
				if (existingEntries.contains (ods->ID_))
				{
					qWarning () << Q_FUNC_INFO
							<< "detected duplicate entry"
							<< ods->ID_;
					continue;
				}

				existingEntries << ods->ID_;
				acc->CreateFromODS (ods);
			}
		}
	}

	void RosterSaver::scheduleSaveRoster (int hint)
	{
		if (SaveRosterScheduled_)
			return;

		SaveRosterScheduled_ = true;
		QTimer::singleShot (hint,
				this,
				SLOT (saveRoster ()));
	}

	void RosterSaver::saveRoster ()
	{
		SaveRosterScheduled_ = false;

		QFile rosterFile (Util::CreateIfNotExists ("azoth/xoox")
					.absoluteFilePath ("roster.xml"));
		if (!rosterFile.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< rosterFile.fileName ()
					<< rosterFile.errorString ();
			return;
		}

		QXmlStreamWriter w (&rosterFile);
		w.setAutoFormatting (true);
		w.setAutoFormattingIndent (2);
		w.writeStartDocument ();
		w.writeStartElement ("roster");
		w.writeAttribute ("formatversion", "1");
		for (auto accObj : Proto_->GetRegisteredAccounts ())
		{
			auto acc = qobject_cast<IAccount*> (accObj);
			w.writeStartElement ("account");
				w.writeTextElement ("id", acc->GetAccountID ());
				w.writeStartElement ("entries");
				for (auto entryObj : acc->GetCLEntries ())
				{
					const auto entry = qobject_cast<GlooxCLEntry*> (entryObj);
					if (!entry ||
							(entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) != ICLEntry::FPermanentEntry)
						continue;

					Save (entry->ToOfflineDataSource (), &w, Proxy_);
				}
				w.writeEndElement ();
			w.writeEndElement ();
		}
		w.writeEndElement ();
		w.writeEndDocument ();
	}

	void RosterSaver::handleAccount (QObject *account)
	{
		connect (account,
				SIGNAL (gotCLItems (QList<QObject*>)),
				this,
				SLOT (checkItemsInvalidation (QList<QObject*>)));
		connect (account,
				SIGNAL (removedCLItems (QList<QObject*>)),
				this,
				SLOT (checkItemsInvalidation (QList<QObject*>)));
		connect (account,
				SIGNAL (rosterSaveRequested ()),
				this,
				SLOT (scheduleSaveRoster ()));
	}

	void RosterSaver::checkItemsInvalidation (const QList<QObject*>& items)
	{
		if (std::any_of (items.begin (), items.end (), [] (QObject *clEntry)
				{
					if (const auto entry = qobject_cast<GlooxCLEntry*> (clEntry))
					{
						const auto lng = entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity;
						return lng == ICLEntry::FPermanentEntry;
					}
					return false;
				}))
			scheduleSaveRoster (5000);
	}
}
}
}
