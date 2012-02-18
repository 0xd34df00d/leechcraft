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

#include "generalarch.h"
#include <QIcon>
#include "config.h"

namespace LeechCraft
{
namespace VFSContainers
{
namespace GeneralArch
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.VFSContainers.GeneralArch";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "VFS Container GeneralArch";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("VFS container for supporting general archive formats.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<VFS::IContainerEngine*> Plugin::GetContainerEngines () const
	{
		return QList<VFS::IContainerEngine*> ();
	}

	QList< VFS::IProtocolEngine* > Plugin::GetProtocolEngines () const
	{
		return QList<VFS::IProtocolEngine*> ();
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_vfscont_generalarch, LeechCraft::VFSContainers::GeneralArch::Plugin);
