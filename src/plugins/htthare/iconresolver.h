/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/asio/awaitable.hpp>
#include <QObject>
#include <QHash>
#include <QSet>

namespace LC
{
namespace HttHare
{
	class IconResolver : public QObject
	{
	public:
		boost::asio::awaitable<QHash<QString, QByteArray>> Resolve (QSet<QString> mimes, int dim);
	};
}
}
