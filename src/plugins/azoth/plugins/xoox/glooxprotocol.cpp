/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "glooxprotocol.h"
#include <QInputDialog>
#include <QMainWindow>
#include <interfaces/iprotocolplugin.h>
#include "glooxaccount.h"
#include "core.h"

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
					GlooxProtocol::GlooxProtocol (QObject *parent)
					: QObject (parent)
					, ParentProtocolPlugin_ (qobject_cast<IProtocolPlugin*> (parent))
					{
					}

					GlooxProtocol::~GlooxProtocol ()
					{
					}

					QObject* GlooxProtocol::GetObject ()
					{
						return this;
					}

					QList<IAccount*> GlooxProtocol::GetRegisteredAccounts ()
					{
						return QList<IAccount*> ();
					}

					IProtocolPlugin* GlooxProtocol::GetParentProtocolPlugin () const
					{
						return ParentProtocolPlugin_;
					}

					QString GlooxProtocol::GetProtocolName () const
					{
						return tr ("XMPP");
					}

					QByteArray GlooxProtocol::GetProtocolID () const
					{
						return "Xoox.Gloox.XMPP";
					}

					void GlooxProtocol::InitiateAccountRegistration ()
					{
						QString name = QInputDialog::getText (0,
								tr ("LeechCraft"),
								tr ("Enter new account name"));
						if (name.isEmpty ())
							return;

						GlooxAccount *account = new GlooxAccount (name, this);
						account->OpenConfigurationDialog ();
						account->ChangeState (IAccount::SOnline);
					}
				}
			}
		}
	}
}
