/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <optional>
#include <QSize>
#include <QString>
#include <util/sll/eitherfwd.h>

class QByteArray;
class QNetworkReply;
class QNetworkAccessManager;

template<typename, typename>
class QHash;

namespace LC::Imgaste
{
	enum class HostingService
	{
		ImagebinCa,
		PomfCat,
		CatboxMoe,
	};

	struct ImageInfo
	{
		quint64 Size_;
		QSize Dim_;
	};

	struct HostingServiceInfo
	{
		QString Name_;
		std::function<bool (ImageInfo)> Accepts_;
	};

	bool operator< (HostingService, HostingService);
	HostingServiceInfo ToInfo (HostingService);
	std::optional<HostingService> FromString (const QString&);
	QList<HostingService> GetAllServices ();

	struct Worker
	{
		virtual ~Worker () = default;

		struct Error {};

		using Result_t = Util::Either<Error, QString>;

		using Headers_t = QHash<QByteArray, QList<QByteArray>>;

		virtual QNetworkReply* Post (const QByteArray& imageData,
				const QString& format, QNetworkAccessManager *am) const = 0;
		virtual Result_t GetLink (const QString& contents, const Headers_t& headers) const = 0;
	};

	typedef std::unique_ptr<Worker> Worker_ptr;

	Worker_ptr MakeWorker (HostingService);
}
