/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "qobjectrefcast.h"

namespace LC::Util::detail
{
	[[noreturn, gnu::cold, gnu::noinline]]
	void NotifyCastError (const QObject *object, const char *target, const std::source_location& loc)
	{
		const QMessageLogger logger { loc.file_name (), static_cast<int> (loc.line ()), loc.function_name () };
		logger.critical ("unable to cast %s to %s",
				object ? object->metaObject ()->className () : "nullptr",
				target);

		using namespace std::string_literals;
		throw BadQObjectCast { "qobject_ref_cast failed at "s + loc.file_name () + ':' + std::to_string (loc.line ()) };
	}
}
