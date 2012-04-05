/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "importmanager.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/isupportimport.h"
#include "interfaces/azoth/ihistoryplugin.h"
#include "core.h"
#include "accounthandlerchooserdialog.h"

namespace LeechCraft
{
namespace Azoth
{
	ImportManager::ImportManager (QObject *parent)
	: QObject (parent)
	{
	}

	void ImportManager::HandleAccountImport (Entity e)
	{
		const QVariantMap& map = e.Additional_ ["AccountData"].toMap ();
		const QString& protoId = map ["Protocol"].toString ();
		if (protoId.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty protocol id"
					<< map;
			return;
		}

		Q_FOREACH (IProtocol *proto, Core::Instance ().GetProtocols ())
		{
			ISupportImport *isi = qobject_cast<ISupportImport*> (proto->GetObject ());
			if (!isi || isi->GetImportProtocolID () != protoId)
				continue;

			isi->ImportAccount (map);
			break;
		}
	}

	void ImportManager::HandleHistoryImport (Entity e)
	{
		qDebug () << Q_FUNC_INFO;
		const auto& histories = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableTo<IHistoryPlugin*> ();
		if (histories.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no history plugin is present, aborting";
			return;
		}

		IAccount *acc = GetAccountID (e);
		if (!acc)
			return;

		ISupportImport *isi = qobject_cast<ISupportImport*> (acc->GetParentProtocol ());

		QHash<QString, QString> entryIDcache;

		QVariantList history;
		Q_FOREACH (Entity qe, EntityQueues_.take (e.Additional_ ["AccountID"].toString ()))
			history.append (qe.Additional_ ["History"].toList ());
		qDebug () << history.size ();
		Q_FOREACH (const QVariant& lineVar, history)
		{
			QVariantMap histMap = lineVar.toMap ();

			const QString& origID = histMap ["EntryID"].toString ();
			if (entryIDcache.contains (origID))
				histMap ["EntryID"] = entryIDcache [origID];
			else
			{
				const QString& realID = isi->GetEntryID (origID, acc->GetObject ());
				entryIDcache [origID] = realID;
				histMap ["EntryID"] = realID;
			}

			if (histMap ["VisibleName"].toString ().isEmpty ())
				histMap ["VisibleName"] = origID;

			histMap ["AccountID"] = acc->GetAccountID ();

			if (histMap ["MessageType"] == "chat")
				histMap ["MessageType"] = static_cast<int> (IMessage::MTChatMessage);
			else if (histMap ["MessageType"] == "muc")
				histMap ["MessageType"] = static_cast<int> (IMessage::MTMUCMessage);
			else if (histMap ["MessageType"] == "event")
				histMap ["MessageType"] = static_cast<int> (IMessage::MTEventMessage);

			Q_FOREACH (IHistoryPlugin *plugin, histories)
				plugin->AddRawMessage (histMap);
		}
	}

	IAccount* ImportManager::GetAccountID (Entity e)
	{
		const QString& accName = e.Additional_ ["AccountName"].toString ();

		auto accs = Core::Instance ().GetAccounts ([] (IProtocol *proto)
				{ return qobject_cast<ISupportImport*> (proto->GetObject ()); });
		IAccount *acc = 0;
		Q_FOREACH (acc, accs)
			if (acc->GetAccountName () == accName)
				return acc;

		const QString& impId = e.Additional_ ["AccountID"].toString ();

		EntityQueues_ [impId] << e;
		if (EntityQueues_ [impId].size () > 1)
			return 0;

		if (AccID2OurID_.contains (impId))
			return AccID2OurID_ [impId];

		QObjectList accObjs;
		Q_FOREACH (IAccount *ia, accs)
			accObjs << ia->GetObject ();
		AccountHandlerChooserDialog dia (accObjs,
				tr ("Select account to import history from %1 into:").arg (accName));
		if (dia.exec () != QDialog::Accepted)
			return 0;

		acc = qobject_cast<IAccount*> (dia.GetSelectedAccount ());
		AccID2OurID_ [impId] = acc;
		return acc;
	}
}
}
