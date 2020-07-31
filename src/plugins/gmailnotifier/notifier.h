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
#include "convinfo.h"

namespace LC
{
namespace GmailNotifier
{
	class Notifier : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		ConvInfos_t PreviousInfos_;
	public:
		Notifier (ICoreProxy_ptr, QObject* = 0);
	public slots:
		void notifyAbout (const ConvInfos_t&);
	};
}
}
