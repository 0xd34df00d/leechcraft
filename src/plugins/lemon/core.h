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

namespace LC
{
namespace Lemon
{
	class PlatformBackend;
	typedef std::shared_ptr<PlatformBackend> PlatformBackend_ptr;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		PlatformBackend_ptr Backend_;

		Core ();
	public:
		static Core& Instance ();
		void Release ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		PlatformBackend_ptr GetPlatformBackend () const;
	};
}
}
