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

#include "glooxclentry.h"
#include <boost/bind.hpp>
#include <QStringList>
#include <QtDebug>
#include <gloox/rosteritem.h>
#include <gloox/resource.h>
#include <interfaces/iaccount.h>
#include <interfaces/azothcommon.h>
#include "glooxaccount.h"

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
					GlooxCLEntry::GlooxCLEntry (gloox::RosterItem *ri, GlooxAccount *parent)
					: QObject (parent)
					, ParentAccount_ (qobject_cast<IAccount*> (parent))
					, ParentAccountObject_ (parent)
					, RI_ (ri)
					{
					}

					QObject* GlooxCLEntry::GetObject ()
					{
						return this;
					}

					IAccount* GlooxCLEntry::GetParentAccount () const
					{
						return ParentAccount_;
					}

					ICLEntry::Features GlooxCLEntry::GetEntryFeatures () const
					{
						return FPermanentEntry | FSupportsRenames | FIsChat;
					}

					QString GlooxCLEntry::GetEntryName () const
					{
						std::string name = RI_->name ();
						if (!name.size ())
							name = RI_->jid ();
						return QString::fromUtf8 (name.c_str ());
					}

					void GlooxCLEntry::SetEntryName (const QString& name)
					{
						RI_->setName (name.toUtf8 ().constData ());
						ParentAccount_->Synchronize ();
					}

					QByteArray GlooxCLEntry::GetEntryID () const
					{
						return RI_->jid ().c_str ();
					}

					QStringList GlooxCLEntry::Groups () const
					{
						QStringList result;
						Q_FOREACH (const std::string& string, RI_->groups ())
							result << QString::fromUtf8 (string.c_str ());
						return result;
					}

					QStringList GlooxCLEntry::Variants () const
					{
						QStringList result;
						gloox::RosterItem::ResourceMap rmap = RI_->resources ();

						QList<gloox::Resource*> resources;
						for (gloox::RosterItem::ResourceMap::const_iterator i = rmap.begin (),
								end = rmap.end (); i != end; ++i)
							resources << i->second;

						std::sort (resources.begin (), resources.end (),
								boost::bind (std::less<int> (),
										boost::bind (&gloox::Resource::priority, _2),
										boost::bind (&gloox::Resource::priority, _1)));

						Q_FOREACH (gloox::Resource *res, resources)
							result << QString::fromUtf8 (res->message ().c_str ());

						if (result.isEmpty ())
							result << "";

						return result;
					}

					IMessage* GlooxCLEntry::CreateMessage (IMessage::MessageType type,
							const QString& variant, const QString& text)
					{
						IMessage *msg = ParentAccountObject_->CreateMessage (type, variant, text, RI_);
						Messages_ << msg;
						return msg;
					}

					QList<IMessage*> GlooxCLEntry::GetAllMessages() const
					{
						return Messages_;
					}

					EntryStatus GlooxCLEntry::GetStatus () const
					{
						return CurrentStatus_;
					}

					void GlooxCLEntry::SetStatus (const EntryStatus& status)
					{
						if (status == CurrentStatus_)
							return;

						CurrentStatus_ = status;
						emit statusChanged (CurrentStatus_);
					}

					void GlooxCLEntry::ReemitMessage (QObject *msg)
					{
						Messages_ << qobject_cast<IMessage*> (msg);
						emit gotMessage (msg);
					}
				}
			}
		}
	}
}
