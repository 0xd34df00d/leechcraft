/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "y7.h"
#include <QIcon>

namespace LC
{
namespace Y7
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Y7";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Y7";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Windows 7 integration layer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_y7, LC::Y7::Plugin);

