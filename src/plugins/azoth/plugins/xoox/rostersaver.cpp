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
#include <util/sll/debugprinters.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qobjectrefcast.h>
#include <util/sys/paths.h>
#include <interfaces/azoth/iproxyobject.h>
#include "clientconnection.h"
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
		if (const auto result = doc.setContent (&rosterFile); !result)
		{
			qWarning () << "unable to read roster file" << result;
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
			for (const auto& entry : Util::DomChildren (account.firstChildElement ("entries"), "entry"))
			{
				const auto ods = std::make_shared<OfflineDataSource> ();
				Load (ods, entry, Proxy_, acc);
				acc->CreateFromODS (ods);
			}
		}
	}

	void RosterSaver::SaveRoster ()
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
			const auto& acc = qobject_ref_cast<GlooxAccount> (accObj);
			const auto conn = acc.GetClientConnection ();
			if (!conn)
				continue;

			w.writeStartElement ("account");
				w.writeTextElement ("id", acc.GetAccountID ());
				w.writeStartElement ("entries");
				for (const auto entry : conn->GetRosterEntries ())
					if ((entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) == ICLEntry::FPermanentEntry)
						Save (entry->ToOfflineDataSource (), &w, Proxy_);
				w.writeEndElement ();
			w.writeEndElement ();
		}
		w.writeEndElement ();
		w.writeEndDocument ();
	}

	void RosterSaver::handleAccount (QObject *accObj)
	{
		connect (qobject_cast<GlooxAccount*> (accObj),
				&GlooxAccount::rosterSaveRequested,
				this,
				[this]
				{
					if (SaveRosterScheduled_)
						return;

					SaveRosterScheduled_ = true;
					QTimer::singleShot (2000,
							this,
							SLOT (saveRoster ()));
				});
	}
}
}
}
