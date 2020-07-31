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
namespace Util
{
namespace Sync
{
	class Stager;
}
}

struct Entity;

namespace Otlozhu
{
	class TodoManager;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		TodoManager *TodoManager_;
		Util::Sync::Stager *Stager_;

		Core ();
	public:
		static Core& Instance ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		TodoManager* GetTodoManager () const;
#ifdef ENABLE_SYNC
		Util::Sync::Stager* GetStager () const;
#endif
	};
}
}
