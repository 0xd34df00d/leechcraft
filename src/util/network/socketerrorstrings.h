/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractSocket>
#include "networkconfig.h"

class QString;

namespace LC::Util
{
	/** @brief Returns an error string for the given socket error.
	 *
	 * This function returns a human-readable localized string describing
	 * the given socket \em error.
	 *
	 * @param[in] error The socket error to describe.
	 * @return The human-readable localized error string.
	 *
	 * @ingroup NetworkUtil
	 */
	UTIL_NETWORK_API QString GetSocketErrorString (QAbstractSocket::SocketError error);
}
