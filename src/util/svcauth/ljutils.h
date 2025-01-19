/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QByteArray>
#include <QString>
#include <util/threads/coro/taskfwd.h>
#include <util/sll/eitherfwd.h>
#include "svcauthconfig.h"

class QNetworkAccessManager;

namespace LC::Util::LJ
{
	struct ChallengeError
	{
		QString Text_;
	};

	struct RequestChallengeConfig
	{
		QNetworkAccessManager& NAM_;
		QByteArray UserAgent_;
	};

	using RequestChallengeResult = Either<ChallengeError, QString>;

	UTIL_SVCAUTH_API Task<RequestChallengeResult> RequestChallenge (RequestChallengeConfig config);
}
