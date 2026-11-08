/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "contentdisposition.h"
#include <QFileInfo>
#include <QList>
#include <util/sll/qtutil.h>

namespace LC::CSTP
{
	namespace
	{
		QList<QStringView> SplitParams (QStringView header)
		{
			QList<QStringView> result;
			qsizetype start = 0;
			bool inQuotes = false;
			for (qsizetype i = 0; i < header.size (); ++i)
			{
				const auto c = header [i];
				if (inQuotes && c == '\\')
					++i;
				else if (c == '"')
					inQuotes = !inQuotes;
				else if (c == ';' && !inQuotes)
				{
					result << header.sliced (start, i - start);
					start = i + 1;
				}
			}
			result << header.sliced (start);
			return result;
		}

		std::optional<QString> Unquote (QStringView value)
		{
			if (!value.startsWith ('"'))
				return value.toString ();

			QString result;
			for (qsizetype i = 1; i < value.size (); ++i)
			{
				auto c = value [i];
				if (c == '"')
				{
					if (i != value.size () - 1)
						return {};
					return result;
				}

				if (c == '\\' && i + 1 < value.size ())
					c = value [++i];
				result += c;
			}
			return {};
		}

		// RFC 8187: charset'[language]'percent-encoded octets, the charset being case-insensitive
		std::optional<QString> DecodeExtValue (QStringView value)
		{
			const auto charsetEnd = value.indexOf ('\'');
			const auto languageEnd = charsetEnd == -1 ? -1 : value.indexOf ('\'', charsetEnd + 1);
			if (languageEnd == -1)
				return {};

			const auto charset = value.first (charsetEnd);
			const auto& octets = QByteArray::fromPercentEncoding (value.sliced (languageEnd + 1).toUtf8 ());
			if (charset.compare (u"UTF-8"_qsv, Qt::CaseInsensitive) == 0)
				return QString::fromUtf8 (octets);
			if (charset.compare (u"ISO-8859-1"_qsv, Qt::CaseInsensitive) == 0)
				return QString::fromLatin1 (octets);
			return {};
		}

		// RFC 6266: parameter names are case-insensitive, and filename* wins over filename
		QString GetFilename (QStringView header)
		{
			std::optional<QString> filename;
			std::optional<QString> extFilename;
			// the first part is the disposition type itself
			for (const auto param : SplitParams (header).sliced (1))
			{
				const auto eqPos = param.indexOf ('=');
				if (eqPos == -1)
					continue;

				const auto name = param.first (eqPos).trimmed ();
				const auto value = param.sliced (eqPos + 1).trimmed ();
				if (!filename && name.compare (u"filename"_qsv, Qt::CaseInsensitive) == 0)
					filename = Unquote (value);
				else if (!extFilename && name.compare (u"filename*"_qsv, Qt::CaseInsensitive) == 0)
					extFilename = DecodeExtValue (value);
			}

			if (extFilename && !extFilename->isEmpty ())
				return *extFilename;
			return filename.value_or (QString {});
		}
	}

	std::optional<QString> GetSuggestedFilename (QByteArrayView header)
	{
		const auto& name = QFileInfo { GetFilename (QString::fromUtf8 (header)) }.fileName ();
		if (name.isEmpty () || name == "."_ql || name == ".."_ql)
			return {};
		return name;
	}
}
