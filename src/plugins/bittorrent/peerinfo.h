/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QString>
#include <QMetaType>

namespace libtorrent
{
    struct peer_info;
}

namespace LC::BitTorrent
{
	struct PeerInfo
	{
		QString IP_;
		QString Client_;
		int RemoteHas_;
		QString CountryCode_;
		std::shared_ptr<libtorrent::peer_info> PI_;
	};
}

Q_DECLARE_METATYPE (LC::BitTorrent::PeerInfo)
