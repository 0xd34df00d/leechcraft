/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringDecoder>
#include <QStringEncoder>
#include "sysconfig.h"

namespace LC::Util
{
	class UTIL_SYS_API EncodingConverter
	{
		QString Name_;
		QStringEncoder Encoder_;
		QStringDecoder Decoder_;
	public:
		class UnknownEncoding : std::runtime_error
		{
			QString Encoding_;
		public:
			explicit UnknownEncoding (const QString& encoding);

			QString GetEncoding () const;
		};

		EncodingConverter ();
		explicit EncodingConverter (QAnyStringView encoding);

		QString GetName () const;

		QString ToUnicode (const QByteArray& bytes);
		QByteArray FromUnicode (const QString& string);
	};
}
