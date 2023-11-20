/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <variant>
#include <QSize>
#include <QString>
#include <QUrl>
#include <QVector>
#include <util/sll/eitherfwd.h>

class QByteArray;
class QHttpMultiPart;
class QNetworkAccessManager;
class QNetworkReply;

template<typename, typename>
class QHash;

namespace LC::Imgaste
{
	struct ImageInfo
	{
		quint64 Size_ = -1;
		QSize Dim_;
	};

	enum class Format
	{
		JPG,
		PNG,
	};

	using MultipartUploader = std::function<std::unique_ptr<QHttpMultiPart> (const QByteArray&, Format)>;
	using ReplyUploader = std::function<QNetworkReply* (const QByteArray&, Format, QNetworkAccessManager&)>;
	using Uploader = std::variant<MultipartUploader, ReplyUploader>;

	struct HostingService
	{
		struct Error {};
		using Result_t = Util::Either<Error, QString>;

		QString Name_;
		QUrl UploadUrl_;
		std::function<bool (ImageInfo)> Accepts_;
		Uploader Upload_;
		std::function<Result_t (const QString&)> GetLink_;
	};

	const QVector<HostingService>& GetAllServices ();
	QNetworkReply* Post (const HostingService& service, const QByteArray&, Format, QNetworkAccessManager&);
}
