/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "platformbackend.h"
#include <QHash>
#include <libnl3/netlink/route/link.h>

namespace LC
{
namespace Lemon
{
	class LinuxPlatformBackend : public PlatformBackend
	{
		Q_OBJECT

		nl_sock *Rtsock_ = nullptr;
		nl_cache *LinkCache_ = nullptr;

		struct DevInfo
		{
			CurrentTrafficState Traffic_;
		};
		QHash<QString, DevInfo> DevInfos_;
	public:
		LinuxPlatformBackend (QObject* = 0);
		~LinuxPlatformBackend ();

		CurrentTrafficState GetCurrentNumBytes (const QString&) const;
	public slots:
		void update (const QStringList&);
	};
}
}
