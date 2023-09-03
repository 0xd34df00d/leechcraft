/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "sllconfig.h"
#include <stdexcept>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

class QByteArray;

namespace LC::Util
{
	template<typename, typename>
	class Either;

	UTIL_SLL_API Either<QString, QJsonDocument> ToJson (const QByteArray& json);

	class UTIL_SLL_API UnexpectedJson final : public std::runtime_error
	{
	public:
		explicit UnexpectedJson (QJsonValue::Type expected, const QJsonValue&);
		explicit UnexpectedJson (QJsonValue::Type expected, const QJsonDocument&);
	};

	template<QJsonValue::Type Expected>
	auto As (const QJsonValue& value)
	{
		if (value.type () != Expected) [[unlikely]]
			throw UnexpectedJson { Expected, value };

		if constexpr (Expected == QJsonValue::Array)
			return value.toArray ();
		if constexpr (Expected == QJsonValue::Object)
			return value.toObject ();
		if constexpr (Expected == QJsonValue::String)
			return value.toString ();
		if constexpr (Expected == QJsonValue::Double)
			return value.toDouble ();
		if constexpr (Expected == QJsonValue::Bool)
			return value.toBool ();
	}

	template<QJsonValue::Type Expected>
		requires (Expected == QJsonValue::Object || Expected == QJsonValue::Array)
	auto As (const QJsonDocument& doc)
	{
		if constexpr (Expected == QJsonValue::Object)
		{
			if (doc.isObject ())
				return doc.object ();
		}
		else if constexpr (Expected == QJsonValue::Array)
		{
			if (doc.isArray ())
				return doc.array ();
		}

		throw UnexpectedJson { Expected, doc };
	}

}
