/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include "newtorrentparams.h"

namespace LC
{
namespace BitTorrent
{
	class TorrentMaker : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
	public:
		TorrentMaker (const ICoreProxy_ptr&, QObject* = 0);
		void Start (NewTorrentParams);
	private:
		void ReportError (const QString&);
	};
}
}
