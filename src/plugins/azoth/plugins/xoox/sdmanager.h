/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QXmppDiscoveryIq.h>
#include "discomanagerwrapper.h"

class ClientConnection;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class SDManager : public QObject
	{
		Q_OBJECT

		ClientConnection * const Conn_;

		typedef QHash<QString, QHash<QString, QXmppDiscoveryIq>> Cache_t;
		Cache_t Infos_;
		Cache_t Items_;
	public:
		SDManager (ClientConnection*);

		void RequestInfo (DiscoManagerWrapper::DiscoCallback_t callback,
				const QString& jid, const QString& node = QString ());
		void RequestItems (DiscoManagerWrapper::DiscoCallback_t callback,
				const QString& jid, const QString& node = QString ());
	private:
		void CommonDo (Cache_t& cache,
				std::function<void (const QString&, DiscoManagerWrapper::DiscoCallback_t, const QString&)>,
				DiscoManagerWrapper::DiscoCallback_t,
				const QString&, const QString&);
	};
}
}
}
