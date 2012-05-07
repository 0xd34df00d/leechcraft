/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <memory>
#include <QObject>
#include <QSet>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>

namespace LeechCraft
{
namespace Blogique
{
class IPluginProxy;

namespace Metida
{
	class LJBloggingPlatform;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QObjectList BlogPlatformPlugins_;
		std::shared_ptr<LJBloggingPlatform> LJPlatform_;
		QObject *PluginProxy_;

		Core ();
		Q_DISABLE_COPY (Core)
	public:
		static Core& Instance ();

		void SecondInit ();

		void CreateBloggingPlatfroms (QObject *parentPlatform);
		void SetCoreProxy (ICoreProxy_ptr proxy);
		ICoreProxy_ptr GetCoreProxy ();

		QObjectList GetBloggingPlatforms () const;

		void SetPluginProxy (QObject *pluginProxy);
		IPluginProxy* GetPluginProxy ();

	signals:
		void gotEntity (LeechCraft::Entity e);
		void delegateEntity (LeechCraft::Entity e, int *id, QObject **obj);
	};
}
}
}
