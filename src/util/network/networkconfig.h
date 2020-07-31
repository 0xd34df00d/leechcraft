/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>

#if defined(leechcraft_util_network_EXPORTS) || defined(leechcraft_util_network_qt5_EXPORTS)
#  define UTIL_NETWORK_API Q_DECL_EXPORT
#else
#  define UTIL_NETWORK_API Q_DECL_IMPORT
#endif

/** @defgroup NetworkUtil The network utilities
 *
 * @brief Common network-related classes and functions.
 */
