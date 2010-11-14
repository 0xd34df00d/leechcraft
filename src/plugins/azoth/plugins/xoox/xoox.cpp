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

#include "xoox.h"
#include <QIcon>
#include <interfaces/iaccount.h>
#include "glooxprotocol.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				namespace Xoox
				{
					void Plugin::Init (ICoreProxy_ptr proxy)
					{
						GlooxProtocol_.reset (new GlooxProtocol (this));
						connect (GlooxProtocol_.get (),
								SIGNAL (gotEntity (const LeechCraft::Entity&)),
								this,
								SIGNAL (gotEntity (const LeechCraft::Entity&)));
					}

					void Plugin::SecondInit ()
					{
						Q_FOREACH (IAccount *account, GlooxProtocol_->GetRegisteredAccounts ())
							account->ChangeState (SOnline);
					}

					void Plugin::Release ()
					{
						GlooxProtocol_.reset ();
					}

					QByteArray Plugin::GetUniqueID () const
					{
						return "org.LeechCraft.Azoth.Xoox";
					}

					QString Plugin::GetName () const
					{
						return "Xoox";
					}

					QString Plugin::GetInfo () const
					{
						return tr ("XMPP (Jabber) protocol support via Gloox library.");
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
						QSet<QByteArray> classes;
						classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
						return classes;
					}

					QObject* Plugin::GetObject ()
					{
						return this;
					}

					QList<IProtocol*> Plugin::GetProtocols () const
					{
						QList<IProtocol*> result;
						result << qobject_cast<IProtocol*> (GlooxProtocol_.get ());
						return result;
					}
				}
			}
		};
	}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_xoox,
		LeechCraft::Plugins::Azoth::Plugins::Xoox::Plugin);

