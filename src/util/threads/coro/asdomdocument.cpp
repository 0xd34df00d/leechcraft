/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "asdomdocument.h"
#include <QtDebug>

namespace LC::Util
{
	AsDomDocument::AsDomDocument (const QByteArray& data, const QString& errorMessage, const std::source_location& loc)
	: ErrorMessage_ { errorMessage }
	{
		if (!Doc_.setContent (data))
			qWarning () << loc.file_name () << ":" << loc.line () << ":" << loc.function_name () << "failed to parse" << data;
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
