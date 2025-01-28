/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "raiisignalconnection.h"
#include <QObject>

namespace LC::Util
{
	RaiiSignalConnection::RaiiSignalConnection (QMetaObject::Connection conn)
	: Conn_ { conn }
	{
	}

	RaiiSignalConnection::~RaiiSignalConnection ()
	{
		QObject::disconnect (Conn_);
	}

	RaiiSignalConnection::RaiiSignalConnection (RaiiSignalConnection&& other) noexcept
	: Conn_ { std::move (other.Conn_)}
	{
	}

	RaiiSignalConnection& RaiiSignalConnection::operator= (RaiiSignalConnection&& other) noexcept
	{
		Conn_.swap (other.Conn_);
		return *this;
	}

	RaiiSignalConnection& RaiiSignalConnection::operator= (QMetaObject::Connection conn)
	{
		QObject::disconnect (Conn_);
		Conn_ = conn;
		return *this;
	}
}
