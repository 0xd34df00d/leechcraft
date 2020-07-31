/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/core/icoreproxy.h>

namespace Wt
{
	class WServer;
}

namespace LC
{
namespace Util
{
	class AddressesModelManager;
}

namespace Aggregator
{
class IProxyObject;

namespace WebAccess
{
	class ServerManager : public QObject
	{
		Q_OBJECT

		std::shared_ptr<Wt::WServer> Server_;

		Util::AddressesModelManager * const AddrMgr_;
	public:
		ServerManager (IProxyObject*, ICoreProxy_ptr, Util::AddressesModelManager*);
	private Q_SLOTS:
		void reconfigureServer ();
	};
}
}
}
