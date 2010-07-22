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
					{
					}

					QObject* GlooxAccount::GetObject ()
					{
						return this;
					}

					IProtocol* GlooxAccount::GetParentProtocol () const
					{
						return ParentProtocol_;
					}

					IAccount::AccountFeatures GlooxAccount::GetAccountFeatures () const
					{
						return FRenamable | FSupportsXA;
					}

					QList<ICLEntry*> GlooxAccount::GetCLEntries ()
					{
						return QList<ICLEntry*> ();
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
						if (dia->exec () == QDialog::Rejected)
							return;

						JID_ = dia->GetJID ();
						Nick_ = dia->GetNick ();
						Resource_ = dia->GetResource ();
						Priority_ = dia->GetPriority ();
					}

					void GlooxAccount::ChangeState (State accState, const QString& status)
					{
						if (accState == IAccount::SOffline)
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
									tr ("LeechCraft"),
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
						gloox::JID jid = gloox::JID (ri->jid ());
						gloox::JID bareJid = jid.bareJID ();
						if (!Sessions_ [bareJid].contains (variant))
						{
							const std::string resource = variant.toUtf8 ().constData ();
							if (ri->resource (resource))
								jid.setResource (resource);

							gloox::MessageSession *ses =
									new gloox::MessageSession (ClientConnection_->GetClient (), jid);
							InitializeSession (ses);
							Sessions_ [bareJid] [variant] = ses;
						}

						GlooxMessage *msg = new GlooxMessage (type, IMessage::DOut,
								ClientConnection_->GetCLEntry (bareJid),
								Sessions_ [bareJid] [variant]);
						msg->SetBody (body);
						msg->SetDateTime (QDateTime::currentDateTime ());
						return msg;
					}

					void GlooxAccount::handleMessage (const gloox::Message& msg,
							gloox::MessageSession *ses)
					{
						gloox::JID bareJid = msg.from ().bareJID ();
						QString variant = QString::fromUtf8 (msg.from ().resource ().c_str ());
						if (!Sessions_ [bareJid].contains (variant))
						{
							InitializeSession (ses);
							Sessions_ [bareJid] [variant] = ses;
						}

						GlooxCLEntry *entry = ClientConnection_->GetCLEntry (bareJid);
						if (!entry)
						{
							qWarning () << Q_FUNC_INFO
									<< "no JID for msg from"
									<< bareJid.full ().c_str ();
							return;
						}

						GlooxMessage *gm = new GlooxMessage (msg, entry, ses);

						entry->ReemitMessage (gm);
					}

					void GlooxAccount::handleGotRosterItems (const QList<QObject*>& items)
					{
						emit gotCLItems (items);
					}

					void GlooxAccount::InitializeSession (gloox::MessageSession *ses)
					{
						ses->registerMessageHandler (this);
					}
				}
			}
		}
	}
}
