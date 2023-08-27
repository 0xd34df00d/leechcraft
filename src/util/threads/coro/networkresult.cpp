/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "networkresult.h"
#include <util/sll/visitor.h>

namespace LC::Util
{
	NetworkReplyErrorException::NetworkReplyErrorException (NetworkReplyError error)
	: std::runtime_error { "network reply returned an error: " + error.ErrorText_.toStdString () }
	, Error_ { std::move (error) }
	{
	}

	const NetworkReplyError& NetworkReplyErrorException::GetError () const
	{
		return Error_;
	}

	QDebug operator<< (QDebug dbg, const NetworkReplyErrorException& exception)
	{
		QDebugStateSaver saver { dbg };

		const auto& error = exception.GetError ();
		dbg.nospace () << "{ url: " << error.Reply_.url ()
				<< "; error: " << error.Error_
				<< "; text: " << error.ErrorText_
				<< " }";

		return dbg;
	}

	QByteArray NetworkResult::GetReplyData () const
	{
		return Visit (*this,
				[] (const NetworkReplySuccess& success) { return success.Data_; },
				[] (const NetworkReplyError& error) -> QByteArray { throw NetworkReplyErrorException { error }; });
	}
}
