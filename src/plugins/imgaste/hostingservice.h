/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
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

	struct HostingService
	{
		virtual ~HostingService () = default;

		HostingService () = default;

		HostingService (const HostingService&) = delete;
		HostingService (HostingService&&) = delete;
		HostingService& operator= (const HostingService&) = delete;
		HostingService& operator= (HostingService&&) = delete;

		virtual QString GetName () const = 0;

		virtual bool Accepts (const ImageInfo&) const = 0;

		struct Error {};

		using Result_t = Util::Either<Error, QString>;

		using Headers_t = QHash<QByteArray, QList<QByteArray>>;

		virtual QNetworkReply* Post (const QByteArray& imageData,
				Format format, QNetworkAccessManager *am) const = 0;

		virtual Result_t GetLink (const QString& contents, const Headers_t& headers) const = 0;
	};

	Q_DECL_EXPORT const QList<std::shared_ptr<HostingService>>& GetAllServices ();
}
