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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXPROTOCOL_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXPROTOCOL_H
#include <QObject>
#include "interfaces/iprotocol.h"

namespace LeechCraft
{
	struct Entity;

	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				namespace Xoox
				{
					class GlooxAccount;

					class GlooxProtocol : public QObject
										, public IProtocol
					{
						Q_OBJECT
						Q_INTERFACES (LeechCraft::Plugins::Azoth::Plugins::IProtocol);

						IProtocolPlugin *ParentProtocolPlugin_;
						QList<GlooxAccount*> Accounts_;
					public:
						GlooxProtocol (QObject*);
						virtual ~GlooxProtocol ();

						QObject* GetObject ();
						ProtocolFeatures GetFeatures () const;
						QList<IAccount*> GetRegisteredAccounts ();
						IProtocolPlugin* GetParentProtocolPlugin () const;
						QString GetProtocolName () const;
						QByteArray GetProtocolID () const;
						void InitiateAccountRegistration ();
						void InitiateMUCJoin ();
					private:
						void SaveAccounts () const;
						void RestoreAccounts ();
					signals:
						void accountAdded (QObject*);
						void gotEntity (const LeechCraft::Entity&);
					};
				}
			}
		}
	}
}

#endif
