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
#include <QtDebug>
#include <interfaces/iplugin2.h>
#include "interfaces/iprotocolplugin.h"
#include "interfaces/iprotocol.h"
#include "interfaces/iaccount.h"
#include "interfaces/iclentry.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			Core::Core ()
			: CLModel_ (new QStandardItemModel (this))
			{
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

			QAbstractItemModel* Core::GetCLModel () const
			{
				return CLModel_;
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
					QList<QAction*> bunch;
					Q_FOREACH (Plugins::IProtocol *proto, ipp->GetProtocols ())
					{
						QAction *accountCreator = new QAction (icon,
								proto->GetProtocolName (), this);
						accountCreator->setData (QVariant::fromValue<QObject*> (proto->GetObject ()));
						connect (accountCreator,
								SIGNAL (triggered ()),
								this,
								SLOT (handleAccountCreatorTriggered ()));

						bunch << accountCreator;

						Q_FOREACH (Plugins::IAccount *account,
								proto->GetRegisteredAccounts ())
							addAccount (account->GetObject ());

						connect (proto->GetObject (),
								SIGNAL (accountAdded (QObject*)),
								this,
								SLOT (addAccount (QObject*)));
					}

					emit accountCreatorActionsAdded (bunch);

					AccountCreatorActions_ += bunch;
				}
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
						Account2Category2Item_ [account] [cat] = catItem;
						account->appendRow (catItem);
					}

					result << Account2Category2Item_ [account] [cat];
				}

				return result;
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
				accItem->setData (QVariant::fromValue<QObject*> (account->GetObject ()),
						CLRAccountObject);
				CLModel_->appendRow (accItem);

				accItem->setEditable (false);

				QList<QStandardItem*> clItems;
				Q_FOREACH (Plugins::ICLEntry *clEntry,
						account->GetCLEntries ())
				{
					QList<QStandardItem*> catItems =
							GetCategoriesItems (clEntry->Groups (), accItem);
					Q_FOREACH (QStandardItem *catItem, catItems)
					{
						QStandardItem *clItem = new QStandardItem (clEntry->GetEntryName ());
						clItem->setEditable (false);
						clItem->setData (QVariant::fromValue<QObject*> (account->GetObject ()),
								CLRAccountObject);
						clItem->setData (QVariant::fromValue<QObject*> (clEntry->GetObject ()),
								CLREntryObject);
						catItem->appendRow (clItem);
					}
				}

				connect (accObject,
						SIGNAL (gotCLItems (const QList<QObject*>&)),
						this,
						SLOT (handleGotCLItems (const QList<QObject*>&)));
			}

			void Core::handleGotCLItems (const QList<QObject*>& items)
			{
				QMap<QObject*, QStandardItem*> accountItemCache;
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

					QStandardItem *accountItem = 0;
					if (accountItemCache.contains (accountObj))
						accountItem = accountItemCache [accountObj];
					else
						for (int i = 0, size = CLModel_->rowCount ();
								i < size; ++i)
							if (CLModel_->item (i)->
										data (CLRAccountObject).value<QObject*> () ==
									accountObj)
							{
								accountItem = CLModel_->item (i);
								accountItemCache [accountObj] = accountItem;
								break;
							}

					if (!accountItem)
					{
						qWarning () << Q_FUNC_INFO
								<< "could not find account item for"
								<< item
								<< accountObj;
						break;
					}

					QList<QStandardItem*> catItems =
							GetCategoriesItems (entry->Groups (), accountItem);
					Q_FOREACH (QStandardItem *catItem, catItems)
					{
						QStandardItem *clItem = new QStandardItem (entry->GetEntryName ());
						clItem->setEditable (false);
						clItem->setData (QVariant::fromValue<QObject*> (item),
								CLREntryObject);
						clItem->setData (QVariant::fromValue<QObject*> (accountObj),
								CLRAccountObject);
						catItem->appendRow (clItem);
					}
				}
			}
		};
	};
};

