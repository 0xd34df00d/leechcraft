/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QByteArray>

class QHttpMultiPart;

namespace LC::Imgaste
{
	enum class Format;

	QByteArray GetMimeType (Format fmt);

	struct File
	{
		Format Format_;
		QByteArray FieldName_;
		QByteArray Data_;
	};

	using Params_t = std::initializer_list<std::pair<QByteArray, QByteArray>>;

	struct MultipartRequest
	{
		QByteArray Boundary_;
		QByteArray Data_;
	};

	std::unique_ptr<QHttpMultiPart> BuildRequest (const File& file);
	std::unique_ptr<QHttpMultiPart> BuildRequest (const Params_t& params, const File& file);
}
