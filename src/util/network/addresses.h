/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QMetaType>
#include "networkconfig.h"

class QHostAddress;

namespace LC::Util
{
	typedef QList<QPair<QString, QString>> AddrList_t;

	/** @brief Returns all local addresses.
	 *
	 * This function returns all local addresses in the UP state,
	 * serialized into a human-readable string and paired with the given
	 * given \em port.
	 *
	 * The local addresses are the ones in the following subnets:
	 * - 10.0.0.0/8
	 * - 172.16.0.0/12
	 * - 192.168.0.0/16
	 *
	 * @param[in] port The port to pair.
	 *
	 * @return The local addresses.
	 *
	 * @ingroup NetworkUtil
	 */
	UTIL_NETWORK_API AddrList_t GetLocalAddresses (int port = 0);

	/** @brief Returns all addresses likely accessible "from the outside".
	 *
	 * @return All accessible addresses.
	 *
	 * @ingroup NetworkUtil
	 */
	UTIL_NETWORK_API QList<QHostAddress> GetAllAddresses ();
}

Q_DECLARE_METATYPE (LC::Util::AddrList_t)
