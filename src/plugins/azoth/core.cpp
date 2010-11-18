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

#include "core.h"
#include <QIcon>
#include <QAction>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDir>
#include <QtDebug>
#include <plugininterface/resourceloader.h>
#include <plugininterface/util.h>
#include <interfaces/iplugin2.h>
#include "interfaces/iprotocolplugin.h"
#include "interfaces/iprotocol.h"
#include "interfaces/iaccount.h"
#include "interfaces/iclentry.h"
#include "chattabsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			Core::Core ()
			: CLModel_ (new QStandardItemModel (this))
			, ChatTabsManager_ (new ChatTabsManager (this))
			, CLIconLoader_ (new Util::ResourceLoader ("azoth/iconsets/contactlist/", this))
			{
				CLIconLoader_->AddLocalPrefix ();

				qRegisterMetaType<Plugins::IMessage*> ("LeechCraft::Plugins::Azoth::Plugins::IMessage*");
				qRegisterMetaType<Plugins::IMessage*> ("Plugins::IMessage*");
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			QSet<QByteArray> Core::GetExpectedPluginClasses () const
			{
				QSet<QByteArray> classes;
				classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
				return classes;
			}

			void Core::AddPlugin (QObject *plugin)
			{
				IPlugin2 *plugin2 = qobject_cast<IPlugin2*> (plugin);
				if (!plugin2)
				{
					qWarning () << Q_FUNC_INFO
							<< plugin
							<< "isn't a IPlugin2";
					return;
				}

				QSet<QByteArray> classes = plugin2->GetPluginClasses ();
				if (classes.contains ("org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin"))
					AddProtocolPlugin (plugin);
			}

			const QObjectList& Core::GetProtocolPlugins () const
			{
				return ProtocolPlugins_;
			}

			QList<QAction*> Core::GetAccountCreatorActions () const
			{
				return AccountCreatorActions_;
			}

			QList<QAction*> Core::GetMUCJoinActions () const
			{
				return MUCJoinActions_;
			}

			QAbstractItemModel* Core::GetCLModel () const
			{
				return CLModel_;
			}

			ChatTabsManager* Core::GetChatTabsManager () const
			{
				return ChatTabsManager_;
			}

			QList<Plugins::IAccount*> Core::GetAccounts () const
			{
				QList<Plugins::IAccount*> result;
				Q_FOREACH (QObject *protoObj, ProtocolPlugins_)
				{
					Plugins::IProtocolPlugin *protoPlug =
							qobject_cast<Plugins::IProtocolPlugin*> (protoObj);
					Q_FOREACH (Plugins::IProtocol *proto, protoPlug->GetProtocols ())
						result << proto->GetRegisteredAccounts ();
				}
				return result;
			}

			void Core::SendEntity (const LeechCraft::Entity& e)
			{
				emit gotEntity (e);
			}

			void Core::OpenChat (const QModelIndex& contactIndex)
			{
				ChatTabsManager_->OpenChat (contactIndex);
			}

			void Core::AddProtocolPlugin (QObject *plugin)
			{
				Plugins::IProtocolPlugin *ipp =
					qobject_cast<Plugins::IProtocolPlugin*> (plugin);
				if (!ipp)
					qWarning () << Q_FUNC_INFO
						<< "plugin"
						<< plugin
						<< "tells it implements the IProtocolPlugin but cast failed";
				else
				{
					ProtocolPlugins_ << plugin;

					QIcon icon = qobject_cast<IInfo*> (plugin)->GetIcon ();
					QList<QAction*> creators;
					QList<QAction*> mucJoiners;
					Q_FOREACH (Plugins::IProtocol *proto, ipp->GetProtocols ())
					{
						QAction *accountCreator = new QAction (icon,
								proto->GetProtocolName (), this);
						accountCreator->setData (QVariant::fromValue<QObject*> (proto->GetObject ()));
						connect (accountCreator,
								SIGNAL (triggered ()),
								this,
								SLOT (handleAccountCreatorTriggered ()));

						creators << accountCreator;

						if (proto->GetFeatures () & Plugins::IProtocol::PFMUCsJoinable)
						{
							QString text = tr ("Join chatroom (%1)")
									.arg (proto->GetProtocolName ());
							QAction *mucJoiner = new QAction (icon,
									text, this);
							mucJoiner->setData (QVariant::fromValue<QObject*> (proto->GetObject ()));
							connect (mucJoiner,
									 SIGNAL (triggered ()),
									 this,
									 SLOT (handleMucJoinRequested ()));

							mucJoiners << mucJoiner;
						}

						Q_FOREACH (Plugins::IAccount *account,
								proto->GetRegisteredAccounts ())
							addAccount (account->GetObject ());

						connect (proto->GetObject (),
								SIGNAL (accountAdded (QObject*)),
								this,
								SLOT (addAccount (QObject*)));
						connect (proto->GetObject (),
								SIGNAL (accountRemoved (QObject*)),
								this,
								SLOT (handleAccountRemoved (QObject*)));
					}

					if (creators.size ())
					{
						emit accountCreatorActionsAdded (creators);
						AccountCreatorActions_ += creators;
					}

					if (mucJoiners.size ())
					{
						emit mucJoinActionsAdded (mucJoiners);
						MUCJoinActions_ += mucJoiners;
					}
				}
			}

			void Core::AddCLEntry (Plugins::ICLEntry *clEntry,
					QStandardItem *accItem)
			{
				connect (clEntry->GetObject (),
						SIGNAL (statusChanged (const Plugins::EntryStatus&)),
						this,
						SLOT (handleStatusChanged (const Plugins::EntryStatus&)));
				connect (clEntry->GetObject (),
						SIGNAL (gotMessage (QObject*)),
						this,
						SLOT (handleEntryGotMessage (QObject*)));

				QList<QStandardItem*> catItems =
						GetCategoriesItems (clEntry->Groups (), accItem);
				Q_FOREACH (QStandardItem *catItem, catItems)
				{
					QStandardItem *clItem = new QStandardItem (clEntry->GetEntryName ());
					clItem->setEditable (false);
					QObject *accObj = clEntry->GetParentAccount ()->GetObject ();
					clItem->setData (QVariant::fromValue<QObject*> (accObj),
							CLRAccountObject);
					clItem->setData (QVariant::fromValue<QObject*> (clEntry->GetObject ()),
							CLREntryObject);
					clItem->setData (QVariant::fromValue<CLEntryType> (CLETContact),
							CLREntryType);
					catItem->appendRow (clItem);

					Entry2Items_ [clEntry] << clItem;
				}

				HandleStatusChanged (clEntry->GetStatus(), clEntry);
			}

			QList<QStandardItem*> Core::GetCategoriesItems (QStringList cats, QStandardItem *account)
			{
				if (cats.isEmpty ())
					cats << tr ("General");

				QList<QStandardItem*> result;
				Q_FOREACH (const QString& cat, cats)
				{
					if (!Account2Category2Item_ [account].keys ().contains (cat))
					{
						QStandardItem *catItem = new QStandardItem (cat);
						catItem->setEditable (false);
						catItem->setData (account->data (CLRAccountObject), CLRAccountObject);
						catItem->setData (QVariant::fromValue<CLEntryType> (CLETCategory),
								CLREntryType);
						Account2Category2Item_ [account] [cat] = catItem;
						account->appendRow (catItem);
					}

					result << Account2Category2Item_ [account] [cat];
				}

				return result;
			}

			QStandardItem* Core::GetAccountItem (const QObject *accountObj,
					QMap<const QObject*, QStandardItem*>& accountItemCache)
			{
				if (accountItemCache.contains (accountObj))
					return accountItemCache [accountObj];
				else
					for (int i = 0, size = CLModel_->rowCount ();
							i < size; ++i)
						if (CLModel_->item (i)->
									data (CLRAccountObject).value<QObject*> () ==
								accountObj)
						{
							QStandardItem *accountItem = CLModel_->item (i);
							accountItemCache [accountObj] = accountItem;
							return accountItem;
						}
			}

			void Core::HandleStatusChanged (const Plugins::EntryStatus& status,
					Plugins::ICLEntry *entry)
			{
				Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
					item->setIcon (GetIconForState (status.State_));
			}

			QIcon Core::GetIconForState (Plugins::State state) const
			{
				QString iconName;
				switch (state)
				{
				case Plugins::SOnline:
					iconName = "online";
					break;
				case Plugins::SChat:
					iconName = "chatty";
					break;
				case Plugins::SAway:
					iconName = "away";
					break;
				case Plugins::SDND:
					iconName = "dnd";
					break;
				case Plugins::SXA:
					iconName = "xa";
					break;
				case Plugins::SOffline:
					iconName = "offline";
					break;
				default:
					iconName = "perr";
					break;
				}

				QString filename = "oxygen/" + iconName;
				QStringList variants;
				variants << filename + ".svg"
						<< filename + ".png"
						<< filename + ".jpg";

				QString path = CLIconLoader_->GetPath (variants);
				return QIcon (path);
			}

			void Core::handleAccountCreatorTriggered ()
			{
				QAction *sa = qobject_cast<QAction*> (sender ());
				if (!sa)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender is not an action"
							<< sender ();
					return;
				}

				QObject *protoObject = sa->data ().value<QObject*> ();
				if (!protoObject)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender data is not QObject"
							<< sa->data ();
					return;
				}

				Plugins::IProtocol *proto =
						qobject_cast<Plugins::IProtocol*> (protoObject);
				if (!proto)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cast protoObject to proto"
							<< protoObject;
					return;
				}

				proto->InitiateAccountRegistration ();
			}

			void Core::handleMucJoinRequested()
			{
				QAction *sa = qobject_cast<QAction*> (sender ());
				if (!sa)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender is not an action"
							<< sender ();
					return;
				}

				QObject *protoObject = sa->data ().value<QObject*> ();
				if (!protoObject)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender data is not QObject*"
							<< sa->data ();
					return;
				}

				Plugins::IProtocol *proto =
						qobject_cast<Plugins::IProtocol*> (protoObject);
				if (!proto)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to case protoObject to proto"
							<< protoObject;
					return;
				}

				proto->InitiateMUCJoin ();
			}

			void Core::addAccount (QObject *accObject)
			{
				Plugins::IAccount *account =
						qobject_cast<Plugins::IAccount*> (accObject);
				if (!account)
				{
					qWarning () << Q_FUNC_INFO
							<< "account doesn't implement Plugins::IAccount*"
							<< accObject
							<< sender ();
					return;
				}

				QStandardItem *accItem = new QStandardItem (account->GetAccountName ());
				accItem->setData (QVariant::fromValue<QObject*> (accObject),
						CLRAccountObject);
				accItem->setData (QVariant::fromValue<CLEntryType> (CLETAccount),
						CLREntryType);
				CLModel_->appendRow (accItem);

				accItem->setEditable (false);

				QList<QStandardItem*> clItems;
				Q_FOREACH (Plugins::ICLEntry *clEntry,
						account->GetCLEntries ())
					AddCLEntry (clEntry, accItem);

				connect (accObject,
						SIGNAL (gotCLItems (const QList<QObject*>&)),
						this,
						SLOT (handleGotCLItems (const QList<QObject*>&)));
				connect (accObject,
						SIGNAL (removedCLItems (const QList<QObject*>&)),
						this,
						SLOT (handleRemovedCLItems (const QList<QObject*>&)));
			}

			void Core::handleAccountRemoved (QObject *account)
			{
				for (int i = 0; i < CLModel_->rowCount (); ++i)
				{
					QStandardItem *item = CLModel_->item (i);
					QObject *obj = item->data (CLRAccountObject).value<QObject*> ();
					if (obj == account)
					{
						CLModel_->removeRow (i);
						break;
					}
				}
			}

			void Core::handleGotCLItems (const QList<QObject*>& items)
			{
				QMap<const QObject*, QStandardItem*> accountItemCache;
				Q_FOREACH (QObject *item, items)
				{
					Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (item);
					if (!entry)
					{
						qWarning () << Q_FUNC_INFO
								<< item
								<< "is not a valid ICLEntry";
						continue;
					}

					Plugins::IAccount *account = entry->GetParentAccount ();
					if (!account)
					{
						qWarning () << Q_FUNC_INFO
								<< "parent account of"
								<< item
								<< "is null";
						continue;
					}

					QObject *accountObj = account->GetObject ();
					if (!accountObj)
					{
						qWarning () << Q_FUNC_INFO
								<< "account object of"
								<< item
								<< "is null";
						continue;
					}

					QStandardItem *accountItem = GetAccountItem (accountObj, accountItemCache);

					if (!accountItem)
					{
						qWarning () << Q_FUNC_INFO
								<< "could not find account item for"
								<< item
								<< accountObj;
						continue;
					}

					AddCLEntry (entry, accountItem);

					if (entry->GetEntryFeatures () & Plugins::ICLEntry::FIsMUC)
					{
						QStandardItem *item = Entry2Items_ [entry].first ();
						OpenChat (CLModel_->indexFromItem (item));
					}
				}
			}

			void Core::handleRemovedCLItems (const QList<QObject*>& items)
			{
				Q_FOREACH (QObject *clitem, items)
				{
					Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (clitem);
					if (!entry)
					{
						qWarning () << Q_FUNC_INFO
								<< clitem
								<< "is not a valid ICLEntry";
						continue;
					}

					Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
					{
						QStandardItem *parent = item->parent ();
						parent->removeRow (item->row ());

						if (!parent->rowCount ())
						{
							QStandardItem *parentParent = parent->parent ();
							parentParent->removeRow (parent->row ());
						}
					}
				}
			}

			void Core::handleStatusChanged (const Plugins::EntryStatus& status)
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (sender ());
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender is not a ICLEntry:"
							<< sender ();
					return;
				}

				HandleStatusChanged (status, entry);
			}

			void Core::handleEntryGotMessage (QObject *msgObj)
			{
				Plugins::IMessage *msg = qobject_cast<Plugins::IMessage*> (msgObj);
				if (!msg)
				{
					qWarning () << Q_FUNC_INFO
							<< msgObj
							<< "doesn't implement Plugins::IMessage";
					return;
				}

				bool shouldShow = msg->GetDirection () == Plugins::IMessage::DIn &&
						msg->GetMessageType () == Plugins::IMessage::MTChatMessage &&
						!ChatTabsManager_->IsActiveChat (msg->OtherPart ());

				if (shouldShow)
				{
					Entity e = Util::MakeNotification ("Azoth",
							tr ("Incoming chat message from %1.")
								.arg (msg->OtherPart ()->GetEntryName ()),
							PInfo_);
					emit gotEntity (e);
				}
			}
		};
	};
};
