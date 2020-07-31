/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef UTIL_STRUCTURESOPS_H
#define UTIL_STRUCTURESOPS_H
#include <QDataStream>
#include "../interfaces/structures.h"
#include "utilconfig.h"

UTIL_API QDataStream& operator<< (QDataStream& out, const LC::Entity& e);
UTIL_API QDataStream& operator>> (QDataStream& in, LC::Entity& e);

namespace LC
{
	UTIL_API bool operator< (const LC::Entity&, const LC::Entity&);
	UTIL_API bool operator== (const LC::Entity&, const LC::Entity&);
}

#endif

