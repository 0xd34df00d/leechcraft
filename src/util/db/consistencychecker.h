/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QObject>
#include <util/sll/either.h>
#include <util/threads/coro/taskfwd.h>
#include "dbconfig.h"

namespace LC::Util::ConsistencyChecker
{
	struct Failed {};
	struct Succeeded {};
	using CheckResult_t = Either<Failed, Succeeded>;

	UTIL_DB_API Task<CheckResult_t> Check (QString dbPath);

	struct RecoverFinished
	{
		qint64 OldFileSize_;
		qint64 NewFileSize_;
	};

	struct RecoverNoSpace
	{
		qint64 Available_;
		qint64 Expected_;
	};
	struct RecoverTargetExists { QString Target_; };
	struct RecoverOtherFailure { QString Message_; };
	using RecoverFailed = std::variant<RecoverNoSpace, RecoverTargetExists, RecoverOtherFailure>;
	using RecoverResult_t = Either<RecoverFailed, RecoverFinished>;

	UTIL_DB_API Task<RecoverResult_t> Recover (QString dbPath);
	UTIL_DB_API Task<RecoverResult_t> RecoverWithUserInteraction (QString dbPath, QString diaTitle);
}
