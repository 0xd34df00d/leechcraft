/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "asdomdocument.h"
#include <QtDebug>
#include <util/sll/debugprinters.h>
#include <util/sll/logging.h>

namespace LC::Util
{
	AsDomDocument::AsDomDocument (const QByteArray& data, const QString& errorMessage, const std::source_location& loc)
	: ErrorMessage_ { errorMessage }
	{
		if (const auto result = Doc_.setContent (data);
			!result)
			LogAt (loc).warning () << "failed to parse" << result << data;
	}

	bool AsDomDocument::await_ready () const
	{
		return !Doc_.isNull ();
	}

	QDomDocument AsDomDocument::await_resume () const
	{
		return Doc_;
	}
}
