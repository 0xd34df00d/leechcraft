/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "networkconfig.h"

class QSslError;
class QTreeWidgetItem;

namespace LC
{
namespace Util
{
	/** @brief Builds a tree widget representation of the given SSL error.
	 *
	 * This function creates and returns a part of the tree for the given
	 * SSL \em error. The ownership of the tree widget item is passed to
	 * the caller.
	 *
	 * @param[in] error The SSL error whose representation should be built.
	 * @return The tree widget item representing the error.
	 *
	 * @ingroup NetworkUtil
	 */
	UTIL_NETWORK_API QTreeWidgetItem* SslError2TreeItem (const QSslError& error);
}
}
