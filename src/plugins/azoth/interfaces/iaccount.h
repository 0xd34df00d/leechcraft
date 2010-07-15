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

#ifndef PLUGINS_AZOTH_INTERFACES_IACCOUNT_H
#define PLUGINS_AZOTH_INTERFACES_IACCOUNT_H
#include <QFlags>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				class IProtocol;
				class ICLEntry;

				class IAccount
				{
				public:
					virtual ~IAccount () {}

					enum AccountFeature
					{
						FRenamable = 0x01,
						FSupportsXA = 0x02,
						FHasConfigurationDialog = 0x04
					};

					Q_DECLARE_FLAGS (AccountFeatures, AccountFeature);

					enum State
					{
						SOffline,
						SOnline,
						SChat,
						SAway,
						SDND,
						SXA,
						SUnavailable,
						SProbe,
						SError,
						SInvalid
					};

					virtual QObject* GetObject () = 0;
					virtual IProtocol* GetParentProtocol () const = 0;
					virtual AccountFeatures GetAccountFeatures () const = 0;
					virtual QList<ICLEntry*> GetCLEntries () = 0;
					virtual QString GetAccountName () const = 0;
					virtual void RenameAccount (const QString&) = 0;
					virtual QByteArray GetAccountID () const = 0;
					virtual void OpenConfigurationDialog () = 0;
					virtual void ChangeState (State, const QString& = QString ()) = 0;
					virtual void Synchronize () = 0;
				};

				Q_DECLARE_OPERATORS_FOR_FLAGS (IAccount::AccountFeatures);
			}
		}
	}
}

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Azoth::Plugins::IAccount,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IAccount/1.0");


#endif
