/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "json.h"
#include <QJsonDocument>
#include <QtDebug>
#include "either.h"

namespace LC::Util
{
	Either<QString, QJsonDocument> ToJson (const QByteArray& json)
	{
		QJsonParseError error;
		auto doc = QJsonDocument::fromJson (json, &error);
		if (error.error == QJsonParseError::NoError)
			return Either<QString, QJsonDocument>::Right (doc);
		return Either<QString, QJsonDocument>::Left (error.errorString ());
	}

	namespace
	{
		std::string MakeUnexpectedMessage (QJsonValue::Type expected, const auto& value)
		{
			QString result;
			QDebug dbg { &result };
			dbg << "unexpected JSON: expected" << expected << "but got" << value;
			return result.toStdString ();
		}
	}

	UnexpectedJson::UnexpectedJson (QJsonValue::Type expected, const QJsonValue& value)
	: std::runtime_error { MakeUnexpectedMessage (expected, value) }
	{
	}

	UnexpectedJson::UnexpectedJson (QJsonValue::Type expected, const QJsonDocument& doc)
	: std::runtime_error { MakeUnexpectedMessage (expected, doc) }
	{
	}
}
