/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <tuple>
#include <QObject>

class QXmppClient;

class QXmppArchiveManager;
class QXmppBookmarkManager;
class QXmppCallManager;
class QXmppDiscoveryManager;
class QXmppEntityTimeManager;
class QXmppMessageReceiptManager;
class QXmppTransferManager;
class QXmppRosterManager;
class QXmppVCardManager;
class QXmppVersionManager;

namespace LC::Azoth::Xoox
{
	class ClientConnection;

	using SimpleExtensions = std::tuple<
				class AdHocCommandManager*,
				class JabberSearchManager*,
				class LastActivityManager*,
				class LegacyEntityTimeExt*,
				class MsgArchivingManager*,
				class PingManager*,
				class RIEXManager*,
				class XMPPAnnotationsManager*,
				class XMPPBobManager*,
				class XMPPCaptchaManager*,
				QXmppArchiveManager*,
				QXmppBookmarkManager*,
#ifdef ENABLE_MEDIACALLS
				QXmppCallManager*,
#endif
				QXmppMessageReceiptManager*,
				QXmppTransferManager*
			>;

	using DefaultExtensions = std::tuple<
				QXmppDiscoveryManager*,
				QXmppEntityTimeManager*,
				QXmppRosterManager*,
				QXmppVCardManager*,
				QXmppVersionManager*
			>;

	class ClientConnectionExtensionsManager : public QObject
	{
		DefaultExtensions DefaultExtensions_;
		SimpleExtensions SimpleExtensions_;
	public:
		explicit ClientConnectionExtensionsManager (ClientConnection&, QXmppClient&, QObject* = nullptr);

		template<typename T>
		T& Get ()
		{
			if constexpr (HasExt<T> (DefaultExtensions {}))
				return *std::get<T*> (DefaultExtensions_);
			else if constexpr (HasExt<T> (SimpleExtensions {}))
				return *std::get<T*> (SimpleExtensions_);
			else
				static_assert (std::is_same_v<T, struct Dummy>, "Unable to find the given extension type");
		}
	private:
		template<typename T, typename... ExtsTypes>
		constexpr static bool HasExt (const std::tuple<ExtsTypes...>&)
		{
			return (std::is_same_v<T*, ExtsTypes> || ...);
		}
	};
}
