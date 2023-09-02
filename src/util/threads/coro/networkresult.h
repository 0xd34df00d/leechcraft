/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QNetworkReply>
#include "../threadsconfig.h"

namespace LC::Util
{
	struct NetworkReplyError
	{
		QNetworkReply::NetworkError Error_;
		QString ErrorText_;
		QNetworkReply& Reply_;
	};

	UTIL_THREADS_API QDebug operator<< (QDebug debug, const NetworkReplyError&);

	struct NetworkReplySuccess
	{
		QByteArray Data_;
		QNetworkReply& Reply_;
	};

	using NRBase_t = std::variant<NetworkReplyError, NetworkReplySuccess>;

	class UTIL_THREADS_API NetworkReplyErrorException : public std::runtime_error
	{
		NetworkReplyError Error_;
	public:
		explicit NetworkReplyErrorException (NetworkReplyError error);

		const NetworkReplyError& GetError () const;
	};

	UTIL_THREADS_API QDebug operator<< (QDebug debug, const NetworkReplyErrorException&);

	class UTIL_THREADS_API NetworkResult : public NRBase_t
	{
	public:
		using NRBase_t::variant;

		std::optional<NetworkReplyError> IsError () const;
		QByteArray GetReplyData () const;
	};

	UTIL_THREADS_API QDebug operator<< (QDebug debug, const NetworkResult&);
}
