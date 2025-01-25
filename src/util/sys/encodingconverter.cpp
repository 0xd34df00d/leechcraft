/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "encodingconverter.h"

namespace LC::Util
{
	EncodingConverter::UnknownEncoding::UnknownEncoding (const QString& encoding)
	: std::runtime_error { "unknown encoding " + encoding.toStdString () }
	, Encoding_ { encoding }
	{
	}

	QString EncodingConverter::UnknownEncoding::GetEncoding () const
	{
		return Encoding_;
	}

	EncodingConverter::EncodingConverter ()
	: Name_ { "System" }
	, Encoder_ { QStringConverter::System }
	, Decoder_ { QStringConverter::System }
	{
	}

	EncodingConverter::EncodingConverter (QAnyStringView encoding)
	: Name_ { encoding.toString () }
	, Encoder_ { encoding }
	, Decoder_ { encoding }
	{
		if (!Encoder_.isValid () || !Decoder_.isValid ())
			throw UnknownEncoding { Name_ };
	}

	QString EncodingConverter::GetName () const
	{
		return Name_;
	}

	QString EncodingConverter::ToUnicode (const QByteArray& bytes)
	{
		return Decoder_.decode (bytes);
	}

	QByteArray EncodingConverter::FromUnicode (const QString& string)
	{
		return Encoder_.encode (string);
	}
}
