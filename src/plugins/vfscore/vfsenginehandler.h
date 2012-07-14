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

#ifndef PLUGINS_VFSCORE_VFSENGINEHANDLER_H
#define PLUGINS_VFSCORE_VFSENGINEHANLDER_H
#include <QAbstractFileEngineHandler>
#include <interfaces/iinfo.h>

namespace LeechCraft
{
namespace VFS
{
	class IContainerEngine;
	class IProtocolEngine;
}

namespace VFScore
{
	class VFSEngineHandler : public QAbstractFileEngineHandler
	{
		ICoreProxy_ptr Proxy_;

		struct MarkedPath
		{
			QString Type_;
			QString Path_;
			
			MarkedPath ();
			MarkedPath (const QString&, const QString&);
		};
		typedef QList<MarkedPath> PathChain_t;
		
		QList<VFS::IContainerEngine*> Containers_;
		QList<VFS::IProtocolEngine*> Protocols_;
	public:
		VFSEngineHandler (ICoreProxy_ptr);

		QAbstractFileEngine* create (const QString&) const;
	private:
		PathChain_t ParsePath (const QString&) const;
	};
}
}

#endif
