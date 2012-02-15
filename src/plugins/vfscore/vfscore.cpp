/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "vfscore.h"
#include <QIcon>
#include <interfaces/vfs/iengineprovider.h>
#include "vfsenginehandler.h"

namespace LeechCraft
{
namespace VFScore
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Handler_.reset (new VFSEngineHandler (proxy));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.VFScore";
	}

	void Plugin::Release ()
	{
		Handler_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "VFScore";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Core of the VFS subsystem in LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_vfscore, LeechCraft::VFScore::Plugin);
