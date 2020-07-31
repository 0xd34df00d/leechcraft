/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QByteArray>
#include <QtDebug>
#include <tox/tox.h>

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	std::optional<qint32> GetFriendId (const Tox *tox, const QByteArray& pubkey)
	{
		const auto& binPkey = QByteArray::fromHex (pubkey);
		TOX_ERR_FRIEND_BY_PUBLIC_KEY error {};

		const auto res = tox_friend_by_public_key (tox, reinterpret_cast<const uint8_t*> (binPkey.constData ()), &error);
		if (res == UINT32_MAX)
		{
			qWarning () << Q_FUNC_INFO
					<< "failed to get friend by public key"
					<< pubkey
					<< error;
			return {};
		}

		return res;
	}

	QByteArray GetFriendId (const Tox *tox, int32_t friendId)
	{
		std::array<uint8_t, TOX_PUBLIC_KEY_SIZE> clientId;
		TOX_ERR_FRIEND_GET_PUBLIC_KEY error {};

		if (!tox_friend_get_public_key (tox, friendId, clientId.data (), &error))
		{
			qWarning () << Q_FUNC_INFO
					<< "failed to get friend's public key"
					<< friendId
					<< error;
			throw std::runtime_error { "Cannot get friend's pubkey." };
		}

		return ToxId2HR (clientId);
	}
}
}
}
