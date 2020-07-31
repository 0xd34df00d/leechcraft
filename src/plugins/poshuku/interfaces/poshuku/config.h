/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef POSHUKU_CONFIG_H
#define POSHUKU_CONFIG_H
#include <QtGlobal>

#if defined(Q_CC_GNU)

# if defined(leechcraft_poshuku_EXPORTS)
#  define POSHUKU_API __attribute__ ((visibility("default")))
# else
#  define POSHUKU_API
# endif

#elif defined(Q_CC_MSVC)

# if defined(leechcraft_poshuku_EXPORTS)
#  define POSHUKU_API __declspec(dllexport)
# else
#  define POSHUKU_API __declspec(dllimport)
# endif

#else
# define POSHUKU_API
#endif

#endif

