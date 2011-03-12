/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "core.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QDomDocument>
#include <QXmppLogger.h>
#include <plugininterface/util.h>
#include <interfaces/iaccount.h>
#include <interfaces/iproxyobject.h>
#include "glooxprotocol.h"
#include "glooxclentry.h"
#include "glooxaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	Core::Core ()
	: PluginProxy_ (0)
	{
		QXmppLogger::getLogger ()->setLoggingType (QXmppLogger::FileLogging);
		QXmppLogger::getLogger ()->setLogFilePath (Util::CreateIfNotExists ("azoth").filePath ("qxmpp.log"));
		QXmppLogger::getLogger ()->setMessageTypes (QXmppLogger::AnyMessage);
		GlooxProtocol_.reset (new GlooxProtocol (this));
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SecondInit ()
	{
		GlooxProtocol_->SetProxyObject (PluginProxy_);
		GlooxProtocol_->Prepare ();
		LoadRoster ();
		Q_FOREACH (QObject *account,
				GlooxProtocol_->GetRegisteredAccounts ())
			connect (account,
					SIGNAL (gotCLItems (const QList<QObject*>&)),
					this,
					SLOT (handleItemsAdded (const QList<QObject*>&)));
	}

	void Core::Release ()
	{
		GlooxProtocol_.reset ();
	}

	QList<QObject*> Core::GetProtocols () const
	{
		QList<QObject*> result;
		result << qobject_cast<QObject*> (GlooxProtocol_.get ());
		return result;
	}

	void Core::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}

	namespace
	{
		struct EntryData
		{
			QByteArray ID_;
			QString Name_;
		};
	}

	void Core::LoadRoster ()
	{
		QFile rosterFile (Util::CreateIfNotExists ("azoth/xoox")
					.absoluteFilePath ("roster.xml"));
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
		Q_FOREACH (QObject *accObj,
				GlooxProtocol_->GetRegisteredAccounts ())
		{
			GlooxAccount *acc = qobject_cast<GlooxAccount*> (accObj);
			id2account [acc->GetAccountID ()] = acc;
		}

		QDomElement account = root.firstChildElement ("account");
		while (!account.isNull ())
		{
			const QByteArray& id = account.firstChildElement ("id").text ().toUtf8 ();
			if (id.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "empty ID";
				continue;
			}

			if (!id2account.contains (id))
				continue;

			QDomElement entry = account
					.firstChildElement ("entries")
					.firstChildElement ("entry");
			while (!entry.isNull ())
			{
				const QByteArray& entryID =
						QByteArray::fromPercentEncoding (entry
								.firstChildElement ("id").text ().toLatin1 ());
				const QString& name = entry.firstChildElement ("name").text ();

				QStringList groups;
				QDomElement group = entry
						.firstChildElement ("groups")
						.firstChildElement ("group");
				while (!group.isNull ())
				{
					const QString& text = group.text ();
					if (!text.isEmpty ())
						groups << text;
					group = group.nextSiblingElement ("group");
				}

				if (entryID.isEmpty ())
					qWarning () << Q_FUNC_INFO
							<< "entry ID is empty";
				else
				{
					GlooxCLEntry::OfflineDataSource_ptr ods (new GlooxCLEntry::OfflineDataSource);
					ods->Name_ = name;
					ods->ID_ = QString::fromUtf8 (entryID.constData ());
					ods->Groups_ = groups;
					ods->AuthStatus_ = qobject_cast<IProxyObject*> (PluginProxy_)->
							AuthStatusFromString (entry.firstChildElement ("authstatus").text ());
					GlooxCLEntry *clEntry = id2account [id]->CreateFromODS (ods);

					const QString& path = Util::CreateIfNotExists ("azoth/xoox/avatars")
							.absoluteFilePath (entryID.toBase64 ());
					clEntry->SetAvatar (QImage (path, "PNG"));
				}
				entry = entry.nextSiblingElement ("entry");
			}

			account = account.nextSiblingElement ("account");
		}
	}

	void Core::saveRoster ()
	{
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
		Q_FOREACH (QObject *accObj,
				GlooxProtocol_->GetRegisteredAccounts ())
		{
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			w.writeStartElement ("account");
				w.writeTextElement ("id", acc->GetAccountID ());
				w.writeStartElement ("entries");
				Q_FOREACH (QObject *entryObj, acc->GetCLEntries ())
				{
					GlooxCLEntry *entry = qobject_cast<GlooxCLEntry*> (entryObj);
					if (!entry ||
							(entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) != ICLEntry::FPermanentEntry)
						continue;

					w.writeStartElement ("entry");
						w.writeTextElement ("id", entry->GetEntryID ().toUtf8 ().toPercentEncoding ("@"));
						w.writeTextElement ("name", entry->GetEntryName ());
						w.writeTextElement ("authstatus",
								qobject_cast<IProxyObject*> (PluginProxy_)->
									AuthStatusToString (entry->GetAuthStatus ()));
						w.writeStartElement ("groups");
						Q_FOREACH (const QString& group, entry->Groups ())
							w.writeTextElement ("group", group);
						w.writeEndElement ();
					w.writeEndElement ();

					saveAvatarFor (entry);
				}
				w.writeEndElement ();
			w.writeEndElement ();
		}
		w.writeEndElement ();
		w.writeEndDocument ();
	}

	void Core::handleItemsAdded (const QList<QObject*>& items)
	{
		bool shouldSave = false;
		Q_FOREACH (QObject *clEntry, items)
		{
			GlooxCLEntry *entry = qobject_cast<GlooxCLEntry*> (clEntry);
			if (!entry ||
					(entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) != ICLEntry::FPermanentEntry)
				continue;

			shouldSave = true;

			connect (entry,
					SIGNAL (avatarChanged (const QImage&)),
					this,
					SLOT (saveAvatarFor ()),
					Qt::UniqueConnection);
		}

		if (shouldSave)
			saveRoster ();
	}

	void Core::saveAvatarFor (GlooxCLEntry *entry)
	{
		if (!entry)
			entry = qobject_cast<GlooxCLEntry*> (sender ());

		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "null parameter and wrong sender():"
					<< sender ();
			return;
		}

		const QByteArray& filename = entry->GetEntryID ().toUtf8 ().toBase64 ();
		const QString& path = Util::CreateIfNotExists ("azoth/xoox/avatars")
				.absoluteFilePath (filename);
		entry->GetAvatar ().save (path, "PNG");
	}
}
}
}