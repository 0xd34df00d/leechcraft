/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chateventsadapter.h"
#include <util/sll/qobjectrefcast.h>
#include <util/sll/qtutil.h>
#include "util/azoth/util.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucentry.h"
#include "../../xmlsettingsmanager.h"

namespace LC::Azoth
{
	namespace
	{
		bool CheckShow (const char *prop)
		{
			return XmlSettingsManager::Instance ().property (prop).toBool ();
		}

		QString GetDisplayId (const ICLEntry& entry, const QString& variant)
		{
			return variant.isEmpty () ? entry.GetEntryName () : entry.GetEntryName () + '/' + variant;
		}

		class DirectAdapter : public ChatEventsAdapter
		{
			ICLEntry& Entry_;
		public:
			DirectAdapter (ICLEntry& entry, QObject *parent)
			: ChatEventsAdapter { parent }
			, Entry_ { entry }
			{
				auto& emitter = Entry_.GetCLEntryEmitter ();
				connect (&emitter,
						&Emitters::CLEntry::chatPartStateChanged,
						this,
						&DirectAdapter::HandleChatState);
			}

			void HandleChatState (ChatPartState state, const QString& variant)
			{
				if (state != CPSGone || !CheckShow ("ShowEndConversations"))
					return;

				emit gotEvent ({ .Text_ { tr ("%1 has left the conversation").arg (GetDisplayId (Entry_, variant)) }  });
			}
		};

		class MucAdapter : public ChatEventsAdapter
		{
			IMUCEntry& Entry_;
		public:
			MucAdapter (IMUCEntry& entry, QObject *parent)
			: ChatEventsAdapter { parent }
			, Entry_ { entry }
			{
			}
		};
	}

	ChatEventsAdapter* ChatEventsAdapter::MakeFor (ICLEntry& entry, QObject *parent)
	{
		switch (entry.GetEntryType ())
		{
		case ICLEntry::EntryType::Chat:
		case ICLEntry::EntryType::PrivateChat:
		case ICLEntry::EntryType::UnauthEntry:
			return new DirectAdapter { entry, parent };
		case ICLEntry::EntryType::MUC:
			return new MucAdapter { qobject_ref_cast<IMUCEntry> (entry.GetQObject ()), parent };
		}

		qFatal () << "unknown entry type:" << static_cast<int> (entry.GetEntryType ());
	}
}
