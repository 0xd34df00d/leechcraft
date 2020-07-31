/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkProxyFactory>

namespace LC
{
namespace XProxy
{
	class ProxiesStorage;

	class ProxyFactory : public QObject
					   , public QNetworkProxyFactory
	{
		Q_OBJECT

		ProxiesStorage * const Storage_;
	public:
		ProxyFactory (ProxiesStorage*);

		QList<QNetworkProxy> queryProxy (const QNetworkProxyQuery&);
	};
}
}
