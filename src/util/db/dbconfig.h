/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>

#if defined(leechcraft_util_db_EXPORTS)
#  define UTIL_DB_API Q_DECL_EXPORT
#else
#  define UTIL_DB_API Q_DECL_IMPORT
#endif

/** @defgroup DbUtil The DB utilities
 *
 * @brief General widgets and classes for databases.
 */
