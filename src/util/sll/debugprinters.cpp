/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "debugprinters.h"
#include <QtDebug>

QDebug operator<< (QDebug debug, const QDomDocument::ParseResult& result)
{
	QDebugStateSaver saver { debug };
	if (result)
		debug << "success";
	else
		debug.nospace () << "failed at "
				<< result.errorLine << ":" << result.errorColumn
				<< ": " << result.errorMessage;
	return debug;
}
