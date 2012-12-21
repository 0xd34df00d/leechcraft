/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include "platformbackend.h"
#include <QHash>
#include <libnl3/netlink/route/link.h>

namespace LeechCraft
{
namespace Lemon
{
	class LinuxPlatformBackend : public PlatformBackend
	{
		Q_OBJECT

		nl_sock *Rtsock_;
		nl_cache *LinkCache_;

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
