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
#include <QSettings>
#include <QCoreApplication>
#include <interfaces/iprotocolplugin.h>
#include "glooxaccount.h"
#include "core.h"
#include "joingroupchatdialog.h"

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
						RestoreAccounts ();
					}

					GlooxProtocol::~GlooxProtocol ()
					{
					}

					QObject* GlooxProtocol::GetObject ()
					{
						return this;
					}

					IProtocol::ProtocolFeatures GlooxProtocol::GetFeatures() const
					{
						return PFSupportsMUCs | PFMUCsJoinable;
					}

					QList<IAccount*> GlooxProtocol::GetRegisteredAccounts ()
					{
						QList<IAccount*> result;
						Q_FOREACH (GlooxAccount *acc, Accounts_)
							result << static_cast<IAccount*> (acc);
						return result;
					}

					IProtocolPlugin* GlooxProtocol::GetParentProtocolPlugin () const
					{
						return ParentProtocolPlugin_;
					}

					QString GlooxProtocol::GetProtocolName () const
					{
						return "XMPP";
					}

					QByteArray GlooxProtocol::GetProtocolID () const
					{
						return "Xoox.Gloox.XMPP";
					}

					void GlooxProtocol::InitiateAccountRegistration ()
					{
						QString name = QInputDialog::getText (0,
								"LeechCraft",
								tr ("Enter new account name"));
						if (name.isEmpty ())
							return;

						GlooxAccount *account = new GlooxAccount (name, this);
						account->OpenConfigurationDialog ();

						connect (account,
								SIGNAL (accountSettingsChanged ()),
								this,
								SLOT (saveAccounts ()));

						Accounts_ << account;
						saveAccounts ();

						emit accountAdded (account);

						account->ChangeState (SOnline);
					}

					void GlooxProtocol::InitiateMUCJoin ()
					{
						JoinGroupchatDialog dlg (Accounts_);
						if (dlg.exec () != QDialog::Accepted)
							return;

						GlooxAccount *account = dlg.GetSelectedAccount ();
						if (!account)
						{
							qWarning () << Q_FUNC_INFO
									<< "no account has been selected";
							return;
						}

						account->JoinRoom (dlg.GetServer (),
								dlg.GetRoom (), dlg.GetNickname ());
					}

					void GlooxProtocol::saveAccounts () const
					{
						QSettings settings (QSettings::IniFormat, QSettings::UserScope,
								QCoreApplication::organizationName (),
								QCoreApplication::applicationName () + "_Azoth_Xoox_Accounts");
						settings.beginWriteArray ("Accounts");
						for (int i = 0, size = Accounts_.size ();
								i < size; ++i)
						{
							settings.setArrayIndex (i);
							settings.setValue ("SerializedData", Accounts_.at (i)->Serialize ());
						}
						settings.endArray ();
						settings.sync ();
					}

					void GlooxProtocol::RestoreAccounts ()
					{
						QSettings settings (QSettings::IniFormat, QSettings::UserScope,
								QCoreApplication::organizationName (),
								QCoreApplication::applicationName () + "_Azoth_Xoox_Accounts");
						int size = settings.beginReadArray ("Accounts");
						for (int i = 0; i < size; ++i)
						{
							settings.setArrayIndex (i);
							QByteArray data = settings.value ("SerializedData").toByteArray ();
							GlooxAccount *acc = GlooxAccount::Deserialize (data, this);
							if (!acc)
							{
								qWarning () << Q_FUNC_INFO
										<< "unserializable acount"
										<< i;
								continue;
							}

							connect (acc,
									SIGNAL (accountSettingsChanged ()),
									this,
									SLOT (saveAccounts ()));

							Accounts_ << acc;
						}
						settings.endArray ();
					}
				}
			}
		}
	}
}
