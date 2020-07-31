/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "importmanager.h"
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/isupportimport.h"
#include "interfaces/azoth/ihistoryplugin.h"
#include "core.h"
#include "accounthandlerchooserdialog.h"

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

	namespace
	{
		IMessage::Direction GetDirection (const QByteArray& dirStr)
		{
			if (dirStr == "out")
				return IMessage::Direction::Out;
			else if (dirStr == "in")
				return IMessage::Direction::In;

			qWarning () << Q_FUNC_INFO
					<< "unknown direction"
					<< dirStr;
			return IMessage::Direction::In;
		}

		IMessage::Type GetMessageType (const QByteArray& typeStr)
		{
			if (typeStr == "chat")
				return IMessage::Type::ChatMessage;
			else if (typeStr == "muc")
				return IMessage::Type::MUCMessage;
			else if (typeStr == "event")
				return IMessage::Type::EventMessage;

			qWarning () << Q_FUNC_INFO
					<< "unknown type"
					<< typeStr;
			return IMessage::Type::ChatMessage;
		}

		IMessage::EscapePolicy GetEscapePolicy (const QByteArray& polStr)
		{
			if (polStr.isEmpty ())
				return IMessage::EscapePolicy::Escape;
			else if (polStr == "escape")
				return IMessage::EscapePolicy::Escape;
			else if (polStr == "noEscape")
				return IMessage::EscapePolicy::NoEscape;

			qWarning () << Q_FUNC_INFO
					<< "unknown escape policy"
					<< polStr;
			return IMessage::EscapePolicy::Escape;
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

		const auto isi = qobject_cast<ISupportImport*> (acc->GetParentProtocol ());

		QHash<QString, QString> entryIDcache;

		QVariantList history;
		for (const auto& qe : EntityQueues_.take (e.Additional_ ["AccountID"].toString ()))
			history.append (qe.Additional_ ["History"].toList ());

		qDebug () << history.size ();

		struct EntryInfo
		{
			QString VisibleName_;
			QList<HistoryItem> Items_;
		};
		QHash<QString, QHash<QString, EntryInfo>> items;
		for (const auto& lineVar : history)
		{
			const auto& histMap = lineVar.toMap ();

			const auto& origId = histMap ["EntryID"].toString ();
			QString entryId;
			if (entryIDcache.contains (origId))
				entryId = entryIDcache [origId];
			else
			{
				const auto& realId = isi->GetEntryID (origId, acc->GetQObject ());
				entryIDcache [origId] = realId;
				entryId = realId;
			}

			auto visibleName = histMap ["VisibleName"].toString ();
			if (visibleName.isEmpty ())
				visibleName = origId;

			const auto& accId = acc->GetAccountID ();

			const HistoryItem item
			{
				histMap ["DateTime"].toDateTime (),
				GetDirection (histMap ["Direction"].toByteArray ()),
				histMap ["Body"].toString (),
				histMap ["OtherVariant"].toString (),
				GetMessageType (histMap ["Type"].toByteArray ()),
				histMap ["RichBody"].toString (),
				GetEscapePolicy (histMap ["EscapePolicy"].toByteArray ())
			};

			auto& info = items [accId] [entryId];
			info.VisibleName_ = visibleName;
			info.Items_ << item;
		}

		for (const auto& accPair : Util::Stlize (items))
			for (const auto& entryPair : Util::Stlize (accPair.second))
			{
				const auto& info = entryPair.second;
				for (const auto plugin : histories)
					plugin->AddRawMessages (accPair.first,
							entryPair.first, info.VisibleName_, info.Items_);
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
