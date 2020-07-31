/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QIcon>
#include <interfaces/core/icoreproxy.h>

class QStandardItem;

namespace LC
{
namespace HotStreams
{
	class IcecastModel;

	class IcecastFetcher : public QObject
	{
		QIcon RadioIcon_ { ":/hotstreams/resources/images/radio.png" };

		IcecastModel * const Model_;
	public:
		IcecastFetcher (IcecastModel*, const ICoreProxy_ptr&, QObject* = nullptr);
	private:
		void FetchList (const ICoreProxy_ptr&);
		void ParseList ();
	};
}
}
