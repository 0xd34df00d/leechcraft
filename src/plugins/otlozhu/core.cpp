/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"

#ifdef ENABLE_SYNC
#include "stager.h"
#include "stagerhandler.h"
#endif

#include "todomanager.h"
#include "todostorage.h"

namespace LC
{
namespace Otlozhu
{
	Core::Core ()
	: TodoManager_ (new TodoManager ("Default", this))
#ifdef ENABLE_SYNC
	, Stager_ (new Util::Sync::Stager ("org.LeechCraft.Otlozhu", this))
#endif
	{
		connect (TodoManager_,
				SIGNAL (gotEntity (LC::Entity)),
				this,
				SIGNAL (gotEntity (LC::Entity)));

#ifdef ENABLE_SYNC
		auto stagerHandler = new StagerHandler (this);

		auto storage = TodoManager_->GetTodoStorage ();
		connect (storage,
				SIGNAL (itemAdded (int)),
				stagerHandler,
				SLOT (handleItemAdded (int)));
		connect (storage,
				SIGNAL (itemRemoved (int)),
				stagerHandler,
				SLOT (handleItemRemoved (int)));
		connect (storage,
				SIGNAL (itemDiffGenerated (QString, QVariantMap)),
				stagerHandler,
				SLOT (handleItemDiffGenerated (QString, QVariantMap)));
#endif
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	TodoManager* Core::GetTodoManager () const
	{
		return TodoManager_;
	}

#ifdef ENABLE_SYNC
	Util::Sync::Stager* Core::GetStager () const
	{
		return Stager_;
	}
#endif
}
}
