/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "remoter.h"
#include <QTranslator>
#include <QIcon>
#include <plugininterface/util.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Remoter
		{
			void Remoter::Init (ICoreProxy_ptr proxy)
			{
				LeechCraft::Util::InstallTranslator ("remoter");
				Core::Instance ().Init (proxy);
			}

			void Remoter::SecondInit ()
			{
			}

			void Remoter::Release ()
			{
			}

			QString Remoter::GetName () const
			{
				return tr ("Remoter");
			}

			QString Remoter::GetInfo () const
			{
				return tr ("Server providing remote access to other plugins."); 
			}

			QStringList Remoter::Provides () const
			{
				return QStringList ("remoteaccess");
			}

			QStringList Remoter::Needs () const
			{
				return QStringList ();
			}

			QStringList Remoter::Uses () const
			{
				return QStringList ();
			}

			void Remoter::SetProvider (QObject*, const QString&)
			{
			}

			QIcon Remoter::GetIcon () const
			{
				return QIcon (":/resources/images/remoter.svg");
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_remoter, LeechCraft::Plugins::Remoter::Remoter);

