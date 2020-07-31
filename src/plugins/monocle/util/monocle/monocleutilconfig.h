/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>

#ifdef leechcraft_monocle_EXPORTS
#  define MONOCLE_UTIL_API Q_DECL_EXPORT
#else
#  define MONOCLE_UTIL_API Q_DECL_IMPORT
#endif
