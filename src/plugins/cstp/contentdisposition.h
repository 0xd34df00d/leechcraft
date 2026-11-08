/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QString>

namespace LC::CSTP
{
	/** @brief Returns the file name off `Content-Disposition` header.
	 *
	 * The returned name is a single path component: it is never empty,
	 * `.` or `..`, and has no directory part, so it can't place a file
	 * outside the directory it's put into.
	 *
	 * @param header The raw value of the `Content-Disposition` header.
	 * @return The suggested file name, or `std::nullopt` if the header
	 * doesn't suggest a usable one.
	 */
	std::optional<QString> GetSuggestedFilename (QByteArrayView header);
}
