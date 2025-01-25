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
	EncodingConverter::EncodingConverter ()
	: Name_ { "System" }
	, Encoder_ { QStringConverter::System }
	, Decoder_ { QStringConverter::System }
	{
	}

	EncodingConverter::EncodingConverter (QStringView encoding)
	: Name_ { encoding.toString () }
	, Encoder_ { encoding }
	, Decoder_ { encoding }
	{
		if (!Encoder_.isValid () || !Decoder_.isValid ())
			throw UnknownEncoding {};
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
