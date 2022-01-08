/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkReply>
#include <util/sll/either.h>
#include <util/sll/void.h>
#include <util/sll/typelist.h>
#include <util/threads/futures.h>
#include "networkconfig.h"

namespace LC::Util
{
	template<typename F>
	void HandleNetworkReply (QObject *context, QNetworkReply *reply, F f)
	{
		QObject::connect (reply,
				&QNetworkReply::finished,
				context,
				[reply, f]
				{
					reply->deleteLater ();
					f (reply->readAll ());
				});
	}

	template<typename>
	struct ErrorInfo;

	template<typename>
	struct ResultInfo;

	struct UTIL_NETWORK_API ReplyWithHeaders
	{
		QByteArray Data_;
		QHash<QByteArray, QList<QByteArray>> Headers_;

		explicit ReplyWithHeaders (QNetworkReply*);
	};

	struct UTIL_NETWORK_API ReplyError
	{
		QNetworkReply::NetworkError Error_;
		QString ErrorString_;

		QVariant HttpStatusCode_;

		explicit ReplyError (QNetworkReply*);
	};

	template<typename... Args>
	auto HandleReply (QNetworkReply *reply, QObject *context)
	{
		using Err = Find<ErrorInfo, Util::Void, Args...>;
		using Res = Find<ResultInfo, QByteArray, Args...>;

		using Result_t = Util::Either<Err, Res>;
		QFutureInterface<Result_t> promise;
		promise.reportStarted ();

		QObject::connect (reply,
				&QNetworkReply::finished,
				context,
				[promise, reply] () mutable
				{
					reply->deleteLater ();

					if constexpr (std::is_same_v<Res, QByteArray>)
						Util::ReportFutureResult (promise, Result_t::Right (reply->readAll ()));
					else if constexpr (std::is_same_v<Res, ReplyWithHeaders>)
						Util::ReportFutureResult (promise, Result_t::Right (Res { reply }));
					else
						static_assert (std::is_same_v<Res, struct Dummy>, "Unsupported reply type");
				});
		QObject::connect (reply,
				&QNetworkReply::errorOccurred,
				context,
				[promise, reply] () mutable
				{
					reply->deleteLater ();

					auto report = [&] (const Err& val) { Util::ReportFutureResult (promise, Result_t::Left (val)); };

					if constexpr (std::is_same_v<Err, QString>)
						report (reply->errorString ());
					else if constexpr (std::is_same_v<Err, Util::Void>)
						report ({});
					else if constexpr (std::is_same_v<Err, ReplyError>)
						report (Err { reply });
					else
						static_assert (std::is_same_v<Err, struct Dummy>, "Unsupported error type");
				});

		return promise.future ();
	}

	template<typename... Args>
	auto HandleReplySeq (QNetworkReply *reply, QObject *context)
	{
		return Sequence (context, HandleReply<Args...> (reply, context));
	}
}
