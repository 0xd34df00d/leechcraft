/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entitygeneratingpage.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/structures.h>

namespace LC
{
namespace NewLife
{
	EntityGeneratingPage::EntityGeneratingPage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: QWizardPage { parent }
	, Proxy_ { proxy }
	{
	}

	void EntityGeneratingPage::SendEntity (const Entity& entity) const
	{
		Proxy_->GetEntityManager ()->HandleEntity (entity);
	}
}
}
