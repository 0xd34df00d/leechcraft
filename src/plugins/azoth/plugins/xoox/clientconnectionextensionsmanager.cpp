/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clientconnectionextensionsmanager.h"
#include <QXmppClient.h>
#include <QXmppArchiveManager.h>
#include <QXmppBookmarkManager.h>
#include <QXmppCallManager.h>
#include <QXmppDiscoveryManager.h>
#include <QXmppEntityTimeManager.h>
#include <QXmppMessageReceiptManager.h>
#include <QXmppRosterManager.h>
#include <QXmppTransferManager.h>
#include <QXmppVCardManager.h>
#include <QXmppVersionManager.h>
#include "xeps/adhoccommandmanager.h"
#include "xeps/jabbersearchmanager.h"
#include "xeps/lastactivitymanager.h"
#include "xeps/legacyentitytimeext.h"
#include "xeps/msgarchivingmanager.h"
#include "xeps/pingmanager.h"
#include "xeps/riexmanager.h"
#include "xeps/xmppannotationsmanager.h"
#include "xeps/xmppbobmanager.h"
#include "xeps/xmppcaptchamanager.h"

namespace LC::Azoth::Xoox
{
	namespace
	{
		auto MakeDefaultExtensions (QXmppClient& client)
		{
			return std::apply ([&client] (auto... types)
					{
						return DefaultExtensions { client.findExtension<std::remove_pointer_t<decltype (types)>> ()... };
					},
					DefaultExtensions {});
		}

		template<typename T>
		T* MakeForType (ClientConnection& conn)
		{
			if constexpr (std::is_constructible_v<T, ClientConnection*>)
				return new T { &conn };
			else
				return new T {};
		}

		auto MakeSimpleExtensions (ClientConnection& conn)
		{
			return std::apply ([&conn] (auto... types)
					{
						return SimpleExtensions { MakeForType<std::remove_pointer_t<decltype (types)>> (conn)... };
					},
					SimpleExtensions {});
		}
	}

	ClientConnectionExtensionsManager::ClientConnectionExtensionsManager (ClientConnection& conn,
			QXmppClient& client, QObject *parent)
	: QObject { parent }
	, DefaultExtensions_ { MakeDefaultExtensions (client) }
	, SimpleExtensions_ { MakeSimpleExtensions (conn) }
	{
		std::apply ([&client] (auto... exts) { (client.addExtension (exts), ...); },
				SimpleExtensions_);
	}
}
