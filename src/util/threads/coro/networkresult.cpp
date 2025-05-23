/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "networkresult.h"
#include <util/sll/either.h>
#include <util/sll/visitor.h>

namespace LC::Util
{
	QDebug operator<< (QDebug dbg, const NetworkReplyError& error)
	{
		QDebugStateSaver saver { dbg };

		dbg.nospace () << "{ url: " << error.Url_
				<< "; error: " << error.Error_
				<< "; text: " << error.ErrorText_
				<< " }";

		return dbg;
	}

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
		return dbg << exception.GetError ();
	}

	std::optional<NetworkReplyError> NetworkResult::IsError () const
	{
		if (const auto errPtr = std::get_if<NetworkReplyError> (this))
			return *errPtr;
		return {};
	}

	QByteArray NetworkResult::GetReplyData () const
	{
		return Visit (*this,
				[] (const NetworkReplySuccess& success) { return success.Data_; },
				[] (const NetworkReplyError& error) -> QByteArray { throw NetworkReplyErrorException { error }; });
	}

	Either<QString, QByteArray> NetworkResult::ToEither (const std::source_location& loc) const
	{
		using Result_t = Either<QString, QByteArray>;
		return Visit (*this,
				[] (const NetworkReplySuccess& success) { return Result_t { success.Data_ }; },
				[&loc] (const NetworkReplyError& error)
				{
					QMessageLogger { loc.file_name (), static_cast<int> (loc.line ()), loc.function_name () }.warning () << error;
					return Result_t { error.ErrorText_ };
				});
	}

	Either<QString, QByteArray> NetworkResult::ToEither (const QString& errorContext) const
	{
		using Result_t = Either<QString, QByteArray>;
		return Visit (*this,
				[] (const NetworkReplySuccess& success) { return Result_t { success.Data_ }; },
				[&errorContext] (const NetworkReplyError& error)
				{
					qWarning () << errorContext << error;
					return Result_t { error.ErrorText_ };
				});
	}

	QDebug operator<< (QDebug dbg, const NetworkResult& result)
	{
		QDebugStateSaver saver { dbg };

		dbg.noquote ();
		Visit (result,
				[&dbg] (const NetworkReplySuccess& success) { dbg << "success:" << success.Data_; },
				[&dbg] (const NetworkReplyError& error) { dbg << "error:" << error; });

		return dbg;
	}
}
