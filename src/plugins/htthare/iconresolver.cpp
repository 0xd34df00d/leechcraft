/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "iconresolver.h"
#include <boost/asio.hpp>
#include <QImage>
#include <QIcon>
#include <QtDebug>
#include <util/util.h>

namespace LC
{
namespace HttHare
{
	namespace
	{
		QByteArray ResolveMime (QString mimetype, int dim)
		{
			mimetype.replace ('/', '-');
			auto icon = QIcon::fromTheme (mimetype);
			if (icon.isNull ())
			{
				mimetype.replace ("x-", "");
				icon = QIcon::fromTheme (mimetype);
			}

			if (icon.isNull ())
				icon = QIcon::fromTheme ("application-octet-stream");

			return Util::GetAsBase64Src (icon.pixmap (dim, dim).toImage ()).toLatin1 ();
		}
	}

	boost::asio::awaitable<QHash<QString, QByteArray>> IconResolver::Resolve (QSet<QString> mimes, int dim)
	{
		return boost::asio::async_initiate<const boost::asio::use_awaitable_t<>&, void (QHash<QString, QByteArray>)> (
				[this] (auto handler, QSet<QString> mimes, int dim)
				{
					QMetaObject::invokeMethod (this,
							[handler = std::move (handler), mimes = std::move (mimes), dim] () mutable
							{
								QHash<QString, QByteArray> icons;
								for (const auto& mime : mimes)
									icons [mime] = ResolveMime (mime, dim);
								boost::asio::post (boost::asio::append (std::move (handler), std::move (icons)));
							},
							Qt::QueuedConnection);
				},
				boost::asio::use_awaitable,
				std::move (mimes),
				dim);
	}
}
}
