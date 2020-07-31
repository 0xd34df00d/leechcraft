/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDBusAbstractAdaptor>

class ICoreProxy;

namespace LC
{
namespace DBus
{
	class CoreProxyServerWrapper : public QDBusAbstractAdaptor
	{
		Q_OBJECT
		Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.ICoreProxy")

		ICoreProxy * const W_;
	public:
		CoreProxyServerWrapper (ICoreProxy*, QObject*);
	};
}
}
