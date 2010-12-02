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

#include "chathistory.h"
#include <QIcon>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				namespace ChatHistory
				{
					void Plugin::Init (ICoreProxy_ptr proxy)
					{
					}

					void Plugin::SecondInit ()
					{
					}

					QByteArray Plugin::GetUniqueID () const
					{
						return "org.LeechCraft.Azoth.ChatHistory";
					}

					void Plugin::Release ()
					{
					}

					QString Plugin::GetName () const
					{
						return "Azoth ChatHistory";
					}

					QString Plugin::GetInfo () const
					{
						return tr ("Stores message history in Azoth.");
					}

					QIcon Plugin::GetIcon () const
					{
						return QIcon ();
					}

					QStringList Plugin::Provides () const
					{
						return QStringList ();
					}

					QStringList Plugin::Needs () const
					{
						return QStringList ();
					}

					QStringList Plugin::Uses () const
					{
						return QStringList ();
					}

					void Plugin::SetProvider (QObject*, const QString&)
					{
					}

					QSet<QByteArray> Plugin::GetPluginClasses () const
					{
						QSet<QByteArray> result;
						result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
						return result;
					}

					void Plugin::hookMessageCreated (IHookProxy_ptr proxy,
							QObject *chatTab, QObject *message)
					{
					}
				}
			}
		}
	}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_chathistory, LeechCraft::Plugins::Azoth::Plugins::ChatHistory::Plugin);

