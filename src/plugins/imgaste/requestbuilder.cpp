/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "requestbuilder.h"
#include <QHttpMultiPart>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "hostingservice.h"

namespace LC::Imgaste
{
	namespace
	{
		QByteArray GetFormatString (Format fmt)
		{
			switch (fmt)
			{
			case Format::JPG:
				return "image/jpeg"_qba;
			case Format::PNG:
				return "image/png"_qba;
			}

			return {};
		}

		QByteArray GetExtension (Format fmt)
		{
			switch (fmt)
			{
			case Format::JPG:
				return "jpg"_qba;
			case Format::PNG:
				return "png"_qba;
			}

			return {};
		}
	}

	std::unique_ptr<QHttpMultiPart> BuildRequest (const Params_t& params, const File& file)
	{
		auto multipart = std::make_unique<QHttpMultiPart> (QHttpMultiPart::FormDataType);
		for (const auto& [name, value] : params)
		{
			QHttpPart part;
			part.setHeader (QNetworkRequest::ContentDispositionHeader, R"(form-data; name=")" + name + '"');
			part.setBody (value);
			multipart->append (part);
		}

		QHttpPart imagePart;
		imagePart.setHeader (QNetworkRequest::ContentTypeHeader, GetFormatString (file.Format_));
		imagePart.setHeader (QNetworkRequest::ContentDispositionHeader,
				R"(form-data; name=")" + file.FieldName_ +
				R"("; filename="screenshot.)"_qba + GetExtension (file.Format_) + '\"');
		imagePart.setBody (file.Data_);
		multipart->append (imagePart);

		return multipart;
	}
}
