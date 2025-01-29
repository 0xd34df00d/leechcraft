/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <source_location>
#include <QDomDocument>
#include "either.h"

namespace LC::Util
{
	class AsDomDocument
	{
		QDomDocument Doc_;
		QString ErrorMessage_;
	public:
		AsDomDocument (const QByteArray& data,
				const QString& errorMessage,
				const std::source_location& loc = std::source_location::current ());

		bool await_ready () const;

		void await_suspend (auto handle)
		{
			detail::TerminateLeftyCoroutine (handle, ErrorMessage_);
		}

		QDomDocument await_resume () const;
	};
}
