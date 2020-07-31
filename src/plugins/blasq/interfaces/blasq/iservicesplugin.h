/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>

namespace LC
{
namespace Blasq
{
	class IService;

	class IServicesPlugin
	{
	public:
		virtual ~IServicesPlugin () {}

		virtual QList<IService*> GetServices () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Blasq::IServicesPlugin, "org.LeechCraft.Blasq.IServicesPlugin/1.0")
