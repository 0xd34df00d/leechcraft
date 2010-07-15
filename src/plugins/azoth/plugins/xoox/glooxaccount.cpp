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
#include <gloox/client.h>
#include <gloox/message.h>
#include <gloox/messagesession.h>
#include <gloox/error.h>
#include <interfaces/iprotocol.h>
#include "glooxaccountconfigurationdialog.h"
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

					void GlooxAccount::ChangeState (State state, const QString& status)
					{
						if (!Client_)
						{
							// TODO nonmodal
							QString pwd = QInputDialog::getText (0,
									tr ("LeechCraft"),
									tr ("Enter password for %1:").arg (JID_));

							gloox::JID jid ((JID_ + '/' + Resource_).toUtf8 ().constData ());
							Client_.reset (new gloox::Client (jid, pwd.toUtf8 ().constData ()));
							Client_->registerMessageHandler (this);
							Client_->registerConnectionListener (this);
							Client_->connect (false);
						}
					}

					void GlooxAccount::handleMessage (const gloox::Message& stanza,
							gloox::MessageSession *session)
					{
						qDebug () << Q_FUNC_INFO << stanza.body ().c_str ();
					}

					void GlooxAccount::onConnect ()
					{
						qDebug () << Q_FUNC_INFO;
					}

					void GlooxAccount::onDisconnect (gloox::ConnectionError e)
					{
						qWarning () << Q_FUNC_INFO << e << Client_->streamErrorText ().c_str () << Client_->authError ();
					}

					void GlooxAccount::onResourceBind (const std::string& resource)
					{
						qDebug () << Q_FUNC_INFO << resource.c_str ();
					}

					void GlooxAccount::onResourceBindError (const gloox::Error *error)
					{
						qWarning () << Q_FUNC_INFO;
						if (error)
							qWarning () << error->text ().c_str ();
					}

					void GlooxAccount::onSessionCreateError (const gloox::Error *error)
					{
						qWarning () << Q_FUNC_INFO;
						if (error)
							qWarning () << error->text ().c_str ();
					}

					void GlooxAccount::onStreamEvent (gloox::StreamEvent e)
					{
						qDebug () << Q_FUNC_INFO;
					}

					bool GlooxAccount::onTLSConnect (const gloox::CertInfo& info)
					{
						qDebug () << Q_FUNC_INFO << info.server.c_str ();
						return true;
					}
				}
			}
		}
	}
}
