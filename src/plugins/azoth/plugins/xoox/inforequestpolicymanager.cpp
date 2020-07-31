/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "inforequestpolicymanager.h"
#include "entrybase.h"
#include "xmlsettingsmanager.h"
#include "roomclentry.h"
#include "roomhandler.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	InfoRequestPolicyManager::InfoRequestPolicyManager (QObject *parent)
	: QObject (parent)
	{
	}

	bool InfoRequestPolicyManager::IsRequestAllowed (InfoRequest req, EntryBase *entry) const
	{
		switch (entry->GetEntryType ())
		{
		case ICLEntry::EntryType::PrivateChat:
			switch (req)
			{
			case InfoRequest::Version:
			{
				if (!XmlSettingsManager::Instance ().property ("RequestVersionInMUCs").toBool ())
					return false;

				auto room = qobject_cast<RoomCLEntry*> (entry->GetParentCLEntryObject ());
				if (room->GetRoomHandler ()->IsGateway ())
					return false;

				break;
			}
			case InfoRequest::VCard:
				if (!XmlSettingsManager::Instance ().property ("RequestVCardsInMUCs").toBool ())
					return false;
				break;
			}
			break;
		case ICLEntry::EntryType::Chat:
			switch (req)
			{
			case InfoRequest::Version:
				if (!XmlSettingsManager::Instance ().property ("RequestVersion").toBool ())
					return false;
				break;
			case InfoRequest::VCard:
				if (!XmlSettingsManager::Instance ().property ("RequestVCards").toBool ())
					return false;
				break;
			}
			break;
		default:
			break;
		}

		return true;
	}
}
}
}
