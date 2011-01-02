/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

