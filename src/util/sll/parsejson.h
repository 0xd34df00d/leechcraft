/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QByteArray>
#include <QVariant>
#include <QIODevice>
#include <QtDebug>
#include <QJsonDocument>

namespace LC
{
namespace Util
{
	/** @brief Parses JSON content in the given bytearray.
	 *
	 * @param[in] bytes The byte array to parse JSON from.
	 * @param[in] context The context string to be used in logging
	 * messages.
	 * @return The recursive QVariant with JSON contents.
	 */
	inline QVariant ParseJson (const QByteArray& bytes, const char *context)
	{
		QJsonParseError error;
		const auto& result = QJsonDocument::fromJson (bytes, &error).toVariant ();
		if (error.error != QJsonParseError::NoError)
		{
			qWarning () << context
					<< "cannot parse"
					<< error.errorString ();
			return {};
		}
		return result;
	}

	/** @brief Utility function parsing JSON from the \em device.
	 *
	 * This function reads all available data from the \em device and
	 * passes it to the other ParseJson() overload.
	 *
	 * @param[in] device The device from which JSON-encoded data should
	 * be read.
	 * @param[in] context The context string to be used in logging
	 * messages.
	 * @return The recursive QVariant with JSON contents.
	 */
	inline QVariant ParseJson (QIODevice *device, const char *context)
	{
		return ParseJson (device->readAll (), context);
	}
}
}
