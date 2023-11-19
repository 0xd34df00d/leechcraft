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
#include <QVector>
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
		struct Error {};
		using Result_t = Util::Either<Error, QString>;

		QString Name_;
		std::function<bool (ImageInfo)> Accepts_;
		std::function<QNetworkReply* (const QByteArray&, Format, QNetworkAccessManager*)> Post_;
		std::function<Result_t (const QString&)> GetLink_;
	};

	Q_DECL_EXPORT const QVector<HostingService>& GetAllServices ();
}
