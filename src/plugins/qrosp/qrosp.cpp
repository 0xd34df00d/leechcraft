/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "qrosp.h"
#include <QIcon>
#include <QUrl>
#include <QFileInfo>
#include <qross/core/manager.h>
#include "pluginmanager.h"
#include "wrapperobject.h"

Q_DECLARE_METATYPE (QObject**);

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
				PluginManager::Instance ().Release ();
			}

			QByteArray Plugin::GetUniqueID () const
			{
				return "org.LeechCraft.Qrosp";
			}

			QString Plugin::GetName () const
			{
				return "Qrosp";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Makes LeechCraft scriptable using Qross.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon ();
			}

			QStringList Plugin::Provides () const
			{
				return QStringList ("qrosp");
			}

			QList<QObject*> Plugin::GetPlugins ()
			{
				return PluginManager::Instance ().GetPlugins ();
			}

			bool Plugin::CouldHandle (const Entity& entity) const
			{
				QString language = entity.Additional_ ["Language"].toString ().toLower ();
				if (entity.Mime_ != "x-leechcraft/script-wrap-request")
					return false;
				if (!entity.Additional_ ["Object"].value<QObject**> ())
					return false;
				if (!Qross::Manager::self ().interpreters ().contains (language))
					return false;
				if (!entity.Entity_.toUrl ().isValid ())
					return false;
				if (!QFileInfo (entity.Entity_
						.toUrl ().toLocalFile ()).exists ())
					return false;

				return true;
			}

			void Plugin::Handle (Entity entity)
			{
				QString language = entity.Additional_ ["Language"].toString ().toLower ();
				QString path = entity.Entity_.toUrl ().toLocalFile ();

				*entity.Additional_ ["Object"].value<QObject**> () = new WrapperObject (language, path);
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_qrosp, LeechCraft::Plugins::Qrosp::Plugin);

