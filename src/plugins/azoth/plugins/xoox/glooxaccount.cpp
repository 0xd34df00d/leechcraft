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

#include "glooxaccount.h"
#include <memory>
#include <QInputDialog>
#include <gloox/messagesession.h>
#include <gloox/client.h>
#include <gloox/message.h>
#include <interfaces/iprotocol.h>
#include "glooxaccountconfigurationdialog.h"
#include "core.h"
#include "clientconnection.h"
#include "glooxmessage.h"
#include "glooxclentry.h"
#include "roomclentry.h"

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
					GlooxAccount::GlooxAccount (const QString& name,
							QObject *parent)
					: QObject (parent)
					, Name_ (name)
					, ParentProtocol_ (qobject_cast<IProtocol*> (parent))
					, Priority_ (-1)
					{
					}

					QObject* GlooxAccount::GetObject ()
					{
						return this;
					}

					QObject* GlooxAccount::GetParentProtocol () const
					{
						return ParentProtocol_->GetObject ();
					}

					IAccount::AccountFeatures GlooxAccount::GetAccountFeatures () const
					{
						return FRenamable | FSupportsXA;
					}

					QList<QObject*> GlooxAccount::GetCLEntries ()
					{
						return QList<QObject*> ();
					}

					QString GlooxAccount::GetAccountName () const
					{
						return Name_;
					}

					void GlooxAccount::RenameAccount (const QString& name)
					{
						Name_ = name;
					}

					QByteArray GlooxAccount::GetAccountID () const
					{
						return ParentProtocol_->GetProtocolID () + JID_.toUtf8 ();
					}

					void GlooxAccount::OpenConfigurationDialog ()
					{
						// TODO nonmodal
						std::auto_ptr<GlooxAccountConfigurationDialog> dia (new GlooxAccountConfigurationDialog (0));
						if (!JID_.isEmpty ())
							dia->SetJID (JID_);
						if (!Nick_.isEmpty ())
							dia->SetNick (Nick_);
						if (!Resource_.isEmpty ())
							dia->SetResource (Resource_);
						dia->SetPriority (Priority_);
						if (dia->exec () == QDialog::Rejected)
							return;

						JID_ = dia->GetJID ();
						Nick_ = dia->GetNick ();
						Resource_ = dia->GetResource ();
						Priority_ = dia->GetPriority ();

						emit accountSettingsChanged ();

						State state = SOnline;
						QString status;
						if (ClientConnection_)
						{
							const gloox::Presence& pres = ClientConnection_->GetClient ()->presence ();
							state = static_cast<State> (pres.presence ());
							status = QString::fromUtf8 (pres.status ().c_str ());
						}
						ChangeState (state, status);
					}

					void GlooxAccount::ChangeState (State accState, const QString& status)
					{
						if (accState == SOffline)
						{
							ClientConnection_.reset ();
							return;
						}

						struct GlooxAccountState state =
						{
							accState,
							status,
							Priority_
						};

						if (!ClientConnection_)
						{
							// TODO nonmodal
							QString pwd = QInputDialog::getText (0,
									"LeechCraft",
									tr ("Enter password for %1:").arg (JID_));

							gloox::JID jid ((JID_ + '/' + Resource_).toUtf8 ().constData ());
							ClientConnection_.reset (new ClientConnection (jid, pwd, state, this));

							connect (ClientConnection_.get (),
									SIGNAL (gotRosterItems (const QList<QObject*>&)),
									this,
									SLOT (handleGotRosterItems (const QList<QObject*>&)));
						}
						else
							ClientConnection_->SetState (state);
					}

					void GlooxAccount::Synchronize ()
					{
						ClientConnection_->Synchronize ();
					}

					QString GlooxAccount::GetJID () const
					{
						return JID_;
					}

					QString GlooxAccount::GetNick () const
					{
						return Nick_;
					}

					void GlooxAccount::JoinRoom (const QString& server,
							const QString& room, const QString& nick)
					{
						QString jidStr = QString ("%1@%2/%3")
								.arg (room, server, nick);
						gloox::JID roomJID (jidStr.toUtf8 ().constData ());

						RoomCLEntry *entry = ClientConnection_->JoinRoom (roomJID);
						emit gotCLItems (QList<QObject*> () << entry);
					}

					boost::shared_ptr<ClientConnection> GlooxAccount::GetClientConnection () const
					{
						return ClientConnection_;
					}

					QByteArray GlooxAccount::Serialize () const
					{
						quint16 version = 1;

						QByteArray result;
						{
							QDataStream ostr (&result, QIODevice::WriteOnly);
							ostr << version
									<< Name_
									<< JID_
									<< Nick_
									<< Resource_
									<< Priority_;
						}

						return result;
					}

					GlooxAccount* GlooxAccount::Deserialize (const QByteArray& data, QObject *parent)
					{
						quint16 version = 0;

						QDataStream in (data);
						in >> version;

						if (version != 1)
						{
							qWarning () << Q_FUNC_INFO
									<< "unknown version"
									<< version;
							return 0;
						}

						QString name;
						in >> name;
						GlooxAccount *result = new GlooxAccount (name, parent);
						in >> result->JID_
								>> result->Nick_
								>> result->Resource_
								>> result->Priority_;

						return result;
					}

					IMessage* GlooxAccount::CreateMessage (IMessage::MessageType type,
							const QString& variant, const QString& body,
							gloox::RosterItem *ri)
					{
						return ClientConnection_->CreateMessage (type, variant, body, ri);
					}

					void GlooxAccount::HandleEntryRemoved (QObject *entry)
					{
						emit removedCLItems (QObjectList () << entry);
					}

					void GlooxAccount::handleGotRosterItems (const QList<QObject*>& items)
					{
						emit gotCLItems (items);
					}
				}
			}
		}
	}
}
