/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tasks.h"
#include <QAbstractItemModel>
#include <util/sll/prelude.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iinfo.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "core.h"

namespace LC
{
namespace DBusManager
{
	QStringList Tasks::GetHolders () const
	{
		return Util::Map (Core::Instance ().GetProxy ()->GetPluginsManager ()->GetAllCastableRoots<IJobHolder*> (),
				[] (auto plugin) { return qobject_cast<IInfo*> (plugin)->GetName (); });
	}

	Tasks::RowCountResult_t Tasks::RowCount (const QString& name) const
	{
		for (auto plugin : Core::Instance ().GetProxy ()->GetPluginsManager ()->GetAllCastableRoots<IJobHolder*> ())
		{
			if (qobject_cast<IInfo*> (plugin)->GetName () != name)
				continue;

			return RowCountResult_t::Right (qobject_cast<IJobHolder*> (plugin)->GetRepresentation ()->rowCount ());
		}

		return RowCountResult_t::Left (IdentifierNotFound { name });
	}

	Tasks::GetDataResult_t Tasks::GetData (const QString& name, int r, int role) const
	{
		for (auto plugin : Core::Instance ().GetProxy ()->GetPluginsManager ()->GetAllCastableRoots<IJobHolder*> ())
		{
			if (qobject_cast<IInfo*> (plugin)->GetName () != name)
				continue;

			const auto model = qobject_cast<IJobHolder*> (plugin)->GetRepresentation ();

			QVariantList result;
			for (int i = 0, size = model->columnCount (); i < size; ++i)
				result << model->index (r, i).data (role);
			return GetDataResult_t::Right (result);
		}

		return GetDataResult_t::Left (IdentifierNotFound { name });
	}
}
}
