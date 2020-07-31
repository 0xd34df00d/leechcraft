/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "eitherfwd.h"
#include "void.h"
#include "sllconfig.h"

class QByteArray;
class QVariant;
class QString;

namespace LC
{
namespace Util
{
	/** @brief Serializes the given \em var to JSON representation.
	 *
	 * This function abstracts away differences between Qt4 and Qt5. It
	 * uses QJson on Qt4 (don't forget to link to it!) and native JSON
	 * functions on Qt5.
	 *
	 * @param[in] var The recursive variant to be serialized to JSON.
	 * @param[in] compact Whether the output should be compacitified
	 * (this parameter may have no effect).
	 * @return The serialized representation of \em var.
	 */
	UTIL_SLL_API QByteArray SerializeJson (const QVariant& var, bool compact = true);

	UTIL_SLL_API Either<QString, Void> SerializeJsonToFile (const QString& filename,
			const QVariant& var, bool compact = true);
}
}

