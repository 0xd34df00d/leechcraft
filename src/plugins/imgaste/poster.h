/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <variant>
#include <QObject>
#include <QFutureInterface>
#include <QNetworkReply>
#include <util/sll/eitherfwd.h>
#include <interfaces/idatafilter.h>
#include "hostingservice.h"

class QNetworkAccessManager;
class QStandardItemModel;

namespace LC
{
struct Entity;

namespace Imgaste
{
	class Poster : public QObject
	{
		const HostingService& Service_;
	public:
		struct NetworkRequestError
		{
			QUrl OriginalUrl_;
			QNetworkReply::NetworkError NetworkError_;
			std::optional<int> HttpCode_;
			QString ErrorString_;
		};
		using ServiceAPIError = HostingService::Error;

		using Error_t = std::variant<NetworkRequestError, ServiceAPIError>;
		using Result_t = Util::Either<Error_t, QString>;
	private:
		QFutureInterface<Result_t> Promise_;
	public:
		Poster (const HostingService& service,
				const QByteArray& data,
				const QString& format,
				QStandardItemModel* = nullptr,
				QObject *parent = nullptr);

		QFuture<Result_t> GetFuture ();
	};
}
}
