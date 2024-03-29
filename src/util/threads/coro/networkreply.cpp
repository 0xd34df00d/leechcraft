/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "networkreply.h"
#include <QNetworkReply>
#include "networkresult.h"

namespace LC::Util::detail
{
	bool NRAwaiter::await_ready () const noexcept
	{
		return Reply_.isFinished ();
	}

	void NRAwaiter::await_suspend (std::coroutine_handle<> handle) noexcept
	{
		auto finishedConn = QObject::connect (&Reply_,
				&QNetworkReply::finished,
				[=, this]
				{
					Reply_.deleteLater ();
					handle ();
				});
		QObject::connect (&Reply_,
				&QNetworkReply::errorOccurred,
				[=, this]
				{
					QObject::disconnect (finishedConn);
					Reply_.deleteLater ();
					handle ();
				});
	}

	NetworkResult NRAwaiter::await_resume () const noexcept
	{
		if (Reply_.error () != QNetworkReply::NoError)
			return NetworkReplyError { Reply_.error (), Reply_.errorString (), Reply_.url () };

		return NetworkReplySuccess { Reply_.readAll (), Reply_.header (QNetworkRequest::LocationHeader) };
	}
}

namespace LC
{
	UTIL_THREADS_API Util::detail::NRAwaiter operator co_await (QNetworkReply& reply)
	{
		return { reply };
	}
}
