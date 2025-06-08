/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/threads/coro/taskfwd.h>
#include <util/sll/eitherfwd.h>
#include <util/sll/void.h>
#include "dbconfig.h"

namespace LC::Util
{
	Task<Either<QString, Void>> DumpSqlite (QString from, QString to);
}
