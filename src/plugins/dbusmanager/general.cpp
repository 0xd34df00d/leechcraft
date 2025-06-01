/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "general.h"
#include <QBuffer>
#include <QPixmap>
#include <QIcon>
#include <util/sll/prelude.h>
#include <util/sll/either.h>
#include <interfaces/iinfo.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include "core.h"

namespace LC
{
namespace DBusManager
{
	QStringList General::GetLoadedPlugins ()
	{
		return Util::Map (Core::Instance ().GetProxy ()->GetPluginsManager ()->GetAllPlugins (),
				[] (auto plugin) { return qobject_cast<IInfo*> (plugin)->GetName (); });
	}

	General::Description_t General::GetDescription (const QString& name)
	{
		for (const auto plugin : Core::Instance ().GetProxy ()->GetPluginsManager ()->GetAllPlugins ())
		{
			IInfo *ii = qobject_cast<IInfo*> (plugin);
			if (ii->GetName () == name)
				return ii->GetInfo ();
		}

		return Util::Left { IdentifierNotFound { name } };
	}

	General::Icon_t General::GetIcon (const QString& name, int dim)
	{
		for (const auto plugin : Core::Instance ().GetProxy ()->GetPluginsManager ()->GetAllPlugins ())
		{
			IInfo *ii = qobject_cast<IInfo*> (plugin);
			if (ii->GetName () != name)
				continue;

			QIcon icon = ii->GetIcon ();
			QPixmap pixmap = icon.pixmap (dim, dim);
			QBuffer buffer;
			if (!pixmap.save (&buffer, "PNG", 100))
				return Util::Left { SerializationError {} };
			return buffer.data ();
		}

		return Util::Left { IdentifierNotFound { name } };
	}
}
}
