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

#include "vfsenginehandler.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/vfs/iengineprovider.h>

namespace LeechCraft
{
namespace VFScore
{
	VFSEngineHandler::MarkedPath::MarkedPath ()
	{
	}

	VFSEngineHandler::MarkedPath::MarkedPath (const QString& type, const QString& path)
	: Type_ (type)
	, Path_ (path)
	{
	}

	VFSEngineHandler::VFSEngineHandler (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	{
		IPluginsManager *mgr = Proxy_->GetPluginsManager ();
		QList<VFS::IEngineProvider*> providers = mgr->
				GetAllCastableTo<VFS::IEngineProvider*> ();
				
		Q_FOREACH (VFS::IEngineProvider *prov, providers)
		{
			Containers_ += prov->GetContainerEngines ();
			Protocols_ += prov->GetProtocolEngines ();
		}
	}

	QAbstractFileEngine* VFSEngineHandler::create (const QString& filename) const
	{
		const VFSEngineHandler::PathChain_t& chain = ParsePath (filename);
		if (chain.size () == 1 && chain.at (0).Type_ == "file")
			return 0;

		return 0;
	}
	
	VFSEngineHandler::PathChain_t VFSEngineHandler::ParsePath (const QString& filename) const
	{
		const int schemeIdx = filename.indexOf ("://");
		if (schemeIdx == -1)
			return PathChain_t () << MarkedPath ("file", filename);
		
		const int absIdx = filename.lastIndexOf ('!');
		if (absIdx == -1)
		{
			const QString& scheme = filename.left (schemeIdx);
			const QString& path = filename.mid (schemeIdx + 3);
			return PathChain_t () << MarkedPath (scheme, path);
		}
		
		const QString& scheme = filename.left (schemeIdx);
		const QString& path = filename.mid (absIdx + 1);
		
		return ParsePath (filename.mid (schemeIdx + 1, absIdx - schemeIdx))
				<< MarkedPath (scheme, path);
	}
}
}
