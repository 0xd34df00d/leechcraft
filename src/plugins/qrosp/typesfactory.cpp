/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "typesfactory.h"
#include "wrappers/entitywrapper.h"

namespace LC
{
namespace Qrosp
{
	TypesFactory::TypesFactory (QObject *parent)
	: QObject (parent)
	{
	}

	QObject* TypesFactory::Create (const QString& type)
	{
		if (type == "LC::Entity")
			return new EntityWrapper (Entity ());
		else
			return 0;
	}
}
}
