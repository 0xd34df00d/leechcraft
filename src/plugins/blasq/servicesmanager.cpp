/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "servicesmanager.h"
#include "interfaces/blasq/iservicesplugin.h"

namespace LC
{
namespace Blasq
{
	ServicesManager::ServicesManager (QObject *parent)
	: QObject (parent)
	{
	}

	void ServicesManager::AddPlugin (IServicesPlugin *plugin)
	{
		const auto& news = plugin->GetServices ();
		Services_ << news;

		for (auto service : news)
			emit serviceAdded (service);
	}

	const QList<IService*>& ServicesManager::GetServices () const
	{
		return Services_;
	}
}
}
