/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QHash>
#include <libnl3/netlink/route/link.h>
#include "platformbackend.h"

namespace LC::Lemon
{
	class LinuxPlatformBackend : public PlatformBackend
	{
		template<typename T>
		using NlPtr = std::unique_ptr<T, void (*) (T*)>;

		struct SockConn final
		{
			nl_sock& Sock_;

			explicit SockConn (nl_sock&);
			~SockConn ();

			SockConn (const SockConn&) = delete;
			SockConn (SockConn&&) = delete;

			SockConn& operator= (const SockConn&) = delete;
			SockConn& operator= (SockConn&&) = delete;
		};

		NlPtr<nl_sock> Rtsock_;
		SockConn Conn_;
		NlPtr<nl_cache> LinkCache_;

		struct DevInfo
		{
			CurrentTrafficState Traffic_;
		};
		QHash<QString, DevInfo> DevInfos_;
	public:
		explicit LinuxPlatformBackend (QObject* = nullptr);
		~LinuxPlatformBackend () override;

		CurrentTrafficState GetCurrentNumBytes (const QString&) const override;
		void Update (const QStringList&) override;
	};
}
