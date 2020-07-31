/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>

namespace LC
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	class Core : public QObject
	{
		Q_OBJECT
		Q_DISABLE_COPY (Core)

		ICoreProxy_ptr Proxy_;

		Core ();

	public:
		static Core& Instance ();

		void SetProxy (ICoreProxy_ptr proxy);
		ICoreProxy_ptr GetProxy () const;

		void SendEntity (const LC::Entity& e);
	};
}
}
}
