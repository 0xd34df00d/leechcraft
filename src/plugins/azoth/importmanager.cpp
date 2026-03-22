/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "importmanager.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/isupportimport.h"
#include "interfaces/azoth/ihistoryplugin.h"
#include "core.h"
#include "components/dialogs/accounthandlerchooserdialog.h"

namespace LC
{
namespace Azoth
{
	ImportManager::ImportManager (QObject *parent)
	: QObject (parent)
	{
	}

	void ImportManager::HandleAccountImport (Entity e)
	{
		const auto& map = e.Additional_ ["AccountData"].toMap ();
		const auto& protoId = map ["Protocol"].toString ();
		if (protoId.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty protocol id"
					<< map;
			return;
		}

		for (const auto proto : Core::Instance ().GetProtocols ())
		{
			const auto isi = qobject_cast<ISupportImport*> (proto->GetQObject ());
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

		const auto acc = GetAccountID (e);
		if (!acc)
			return;

		QVariantList history;
		for (const auto& qe : EntityQueues_.take (e.Additional_ ["AccountID"].toString ()))
			history.append (qe.Additional_ ["History"].toList ());

		qDebug () << history.size ();

		using namespace History;
		using enum EntryKind;

		const auto ourNick = acc->GetOurNick ();
		const auto accountId = acc->GetAccountID ();
		const auto accountName = acc->GetAccountName ();

		QHash<QString, EntryWithMessages<Chat>> chatEntries;
		QHash<QString, EntryWithMessages<MUC>> mucEntries;

		for (const auto& lineVar : history)
		{
			const auto& histMap = lineVar.toMap ();

			const auto& entryHRId = histMap ["EntryID"].toString ();
			const auto& typeStr = histMap ["MessageType"].toByteArray ();

			if (typeStr == "event")
				continue;

			const auto& visibleName = histMap ["VisibleName"].toString ();
			const auto& otherVariant = histMap ["Variant"].toString ();
			const auto& richBody = histMap ["RichBody"].toString ();

			const bool isMuc = typeStr == "muc";
			const bool isOut = histMap ["Direction"].toByteArray () == "out";

			const auto appendTo = [&]<EntryKind K> (QHash<QString, EntryWithMessages<K>>& hash,
					EntryDescr<K> descr, EntryEndpoint<K> incoming)
			{
				auto it = hash.find (entryHRId);
				if (it == hash.end ())
					it = hash.insert (entryHRId,
					{
						Entry<K>
						{
							.AccountId_ = accountId,
							.AccountName_ = accountName,
							.EntryHumanReadableId_ = entryHRId,
							.Description_ = std::move (descr),
						},
						{},
					});

				auto endpoint = isOut ?
						Endpoint<K> { SelfEndpoint { ourNick, {} } } :
						Endpoint<K> { std::move (incoming) };
				it->second << NewMessage<K>
				{
					.Endpoint_ = std::move (endpoint),
					.TS_ = histMap ["DateTime"].toDateTime (),
					.Body_ = histMap ["Body"].toString (),
					.RichBody_ = richBody.isEmpty () ? std::nullopt : std::optional { richBody },
				};
				return it;
			};

			const auto displayName = visibleName.isEmpty () ? entryHRId : visibleName;
			if (isMuc)
				appendTo (mucEntries,
						EntryDescr<MUC> { displayName },
						EntryEndpoint<MUC> { .Nick_ = otherVariant, .PersistentId_ = {} });
			else
			{
				auto it = appendTo (chatEntries,
						EntryDescr<Chat> { displayName },
						EntryEndpoint<Chat> { .Variant_ = otherVariant.isEmpty () ? std::nullopt : std::optional { otherVariant } });
				if (!visibleName.isEmpty ())
					it->first.Description_.Nick_ = visibleName;
			}
		}

		for (const auto& ewm : chatEntries)
		{
			const SomeEntryWithMessages some { ewm };
			for (const auto plugin : histories)
				plugin->AddMessages (some);
		}
		for (const auto& ewm : mucEntries)
		{
			const SomeEntryWithMessages some { ewm };
			for (const auto plugin : histories)
				plugin->AddMessages (some);
		}
	}

	IAccount* ImportManager::GetAccountID (Entity e)
	{
		const auto& accName = e.Additional_ ["AccountName"].toString ();

		const auto& accs = Core::Instance ().GetAccounts ([] (IProtocol *proto)
				{ return qobject_cast<ISupportImport*> (proto->GetQObject ()); });
		const auto pos = std::find_if (accs.begin (), accs.end (),
				[&accName] (IAccount *acc) { return acc->GetAccountName () == accName; });
		if (pos != accs.end ())
			return *pos;

		const auto& impId = e.Additional_ ["AccountID"].toString ();

		EntityQueues_ [impId] << e;
		if (EntityQueues_ [impId].size () > 1)
			return nullptr;

		if (AccID2OurID_.contains (impId))
			return AccID2OurID_ [impId];

		AccountHandlerChooserDialog dia (accs,
				tr ("Select account to import history from %1 into:").arg (accName));
		if (dia.exec () != QDialog::Accepted)
			return 0;

		const auto acc = dia.GetSelectedAccount ();
		AccID2OurID_ [impId] = acc;
		return acc;
	}
}
}
