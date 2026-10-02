/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <concepts>
#include <optional>
#include <source_location>
#include <QDataStream>
#include "sysconfig.h"

namespace LC::Util
{
	namespace detail
	{
		UTIL_SYS_API void ReportBadVersion (std::source_location loc, qint64 min, qint64 max, qint64 version);
		UTIL_SYS_API void ReportBadStreamStatus (std::source_location loc, QDataStream::Status status);
	}

	template<std::integral T>
	[[nodiscard]]
	std::optional<T> EnsureVersion (QDataStream& in,
			std::type_identity_t<T> min,
			std::type_identity_t<T> max,
			std::source_location loc = std::source_location::current ())
	{
		T version = 0;

		in >> version;
		if (in.status () != QDataStream::Ok)
		{
			detail::ReportBadStreamStatus (loc, in.status ());
			return {};
		}

		if (version < min || version > max)
		{
			detail::ReportBadVersion (loc, min, max, version);
			in.setStatus (QDataStream::ReadCorruptData);
			return {};
		}
		return version;
	}

	extern template UTIL_SYS_API std::optional<quint8> EnsureVersion<quint8> (QDataStream&, quint8, quint8, std::source_location);
	extern template UTIL_SYS_API std::optional<qint8> EnsureVersion<qint8> (QDataStream&, qint8, qint8, std::source_location);
	extern template UTIL_SYS_API std::optional<quint16> EnsureVersion<quint16> (QDataStream&, quint16, quint16, std::source_location);
}
