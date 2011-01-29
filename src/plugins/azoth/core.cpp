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
#include <QMenu>
#include <QMetaMethod>
#include <QInputDialog>
#include <QMainWindow>
#include <QtDebug>
#include <plugininterface/resourceloader.h>
#include <plugininterface/util.h>
#include <plugininterface/defaulthookproxy.h>
#include <interfaces/iplugin2.h>
#include "interfaces/iprotocolplugin.h"
#include "interfaces/iprotocol.h"
#include "interfaces/iaccount.h"
#include "interfaces/iclentry.h"
#include "interfaces/imucentry.h"
#include "interfaces/iauthable.h"
#include "chattabsmanager.h"
#include "pluginmanager.h"
#include "proxyobject.h"
#include "xmlsettingsmanager.h"
#include "joinconferencedialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			Core::Core ()
			: CLModel_ (new QStandardItemModel (this))
			, ChatTabsManager_ (new ChatTabsManager (this))
			, StatusIconLoader_ (new Util::ResourceLoader ("azoth/iconsets/contactlist/", this))
			, ClientIconLoader_ (new Util::ResourceLoader ("azoth/iconsets/clients/", this))
			, PluginManager_ (new PluginManager)
			, PluginProxyObject_ (new ProxyObject)
			{
				connect (ChatTabsManager_,
						SIGNAL (clearUnreadMsgCount (QObject*)),
						this,
						SLOT (handleClearUnreadMsgCount (QObject*)));
				PluginManager_->RegisterHookable (this);

				StatusIconLoader_->AddLocalPrefix ();
				StatusIconLoader_->AddGlobalPrefix ();

				ClientIconLoader_->AddLocalPrefix ();
				ClientIconLoader_->AddGlobalPrefix ();

				qRegisterMetaType<Plugins::IMessage*> ("LeechCraft::Plugins::Azoth::Plugins::IMessage*");
				qRegisterMetaType<Plugins::IMessage*> ("Plugins::IMessage*");

				XmlSettingsManager::Instance ().RegisterObject ("StatusIcons",
						this, "updateStatusIconset");
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::Release ()
			{
				StatusIconLoader_.reset ();
				ClientIconLoader_.reset ();
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			Util::ResourceLoader* Core::GetResourceLoader (Core::ResourceLoaderType type) const
			{
				switch (type)
				{
				case RLTStatusIconLoader:
					return StatusIconLoader_.get ();
				case RLTClientIconLoader:
					return ClientIconLoader_.get ();
				}
			}

			QSet<QByteArray> Core::GetExpectedPluginClasses () const
			{
				QSet<QByteArray> classes;
				classes << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
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

				QByteArray sig = QMetaObject::normalizedSignature ("initPlugin (QObject*)");
				if (plugin->metaObject ()->indexOfMethod (sig) != -1)
					QMetaObject::invokeMethod (plugin,
							"initPlugin",
							Q_ARG (QObject*, PluginProxyObject_.get ()));

				PluginManager_->AddPlugin (plugin);

				QSet<QByteArray> classes = plugin2->GetPluginClasses ();
				if (classes.contains ("org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin"))
					AddProtocolPlugin (plugin);
			}

			void Core::RegisterHookable (QObject *object)
			{
				PluginManager_->RegisterHookable (object);
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
					Q_FOREACH (QObject *protoObj, protoPlug->GetProtocols ())
					{
						Plugins::IProtocol *proto = qobject_cast<Plugins::IProtocol*> (protoObj);
						Q_FOREACH (QObject *accObj, proto->GetRegisteredAccounts ())
						{
							Plugins::IAccount *acc = qobject_cast<Plugins::IAccount*> (accObj);
							if (!acc)
							{
								qWarning () << Q_FUNC_INFO
										<< "account object from protocol"
										<< proto->GetProtocolID ()
										<< "doesn't implement IAccount"
										<< accObj;
								continue;
							}
							result << acc;
						}
					}
				}
				return result;
			}

			QList<Plugins::IProtocol*> Core::GetProtocols () const
			{
				QList<Plugins::IProtocol*> result;
				Q_FOREACH (QObject *protoPlugin, ProtocolPlugins_)
				{
					QObjectList protos = qobject_cast<Plugins::IProtocolPlugin*> (protoPlugin)->GetProtocols ();
					Q_FOREACH (QObject *obj, protos)
						result << qobject_cast<Plugins::IProtocol*> (obj);
				}
				result.removeAll (0);
				return result;
			}

			void Core::SendEntity (const LeechCraft::Entity& e)
			{
				emit gotEntity (e);
			}

			QObject* Core::GetEntry (const QByteArray& id) const
			{
				return ID2Entry_.value (id);
			}

			void Core::OpenChat (const QModelIndex& contactIndex)
			{
				ChatTabsManager_->OpenChat (contactIndex);
			}

			bool Core::ShouldCountUnread (const Plugins::ICLEntry *entry,
					const Plugins::IMessage *msg)
			{
				return !ChatTabsManager_->IsActiveChat (entry) &&
						(msg->GetMessageType () == Plugins::IMessage::MTChatMessage ||
						 msg->GetMessageType () == Plugins::IMessage::MTMUCMessage);
			}

			bool Core::IsHighlightMessage (const Plugins::IMessage *msg)
			{
				Plugins::IMUCEntry *mucEntry =
						qobject_cast<Plugins::IMUCEntry*> (msg->ParentCLEntry ());
				if (!mucEntry)
					return false;

				return msg->GetBody ().contains (mucEntry->GetNick ());
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
					Q_FOREACH (QObject *protoObj, ipp->GetProtocols ())
					{
						Plugins::IProtocol *proto = qobject_cast<Plugins::IProtocol*> (protoObj);
						QAction *accountCreator = new QAction (icon,
								proto->GetProtocolName (), this);
						accountCreator->setData (QVariant::fromValue<QObject*> (proto->GetObject ()));
						connect (accountCreator,
								SIGNAL (triggered ()),
								this,
								SLOT (handleAccountCreatorTriggered ()));

						creators << accountCreator;

						Q_FOREACH (QObject *accObj,
								proto->GetRegisteredAccounts ())
							addAccount (accObj);

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
				}
			}

			void Core::AddCLEntry (Plugins::ICLEntry *clEntry,
					QStandardItem *accItem)
			{
				connect (clEntry->GetObject (),
						SIGNAL (statusChanged (const Plugins::EntryStatus&, const QString&)),
						this,
						SLOT (handleStatusChanged (const Plugins::EntryStatus&, const QString&)));
				connect (clEntry->GetObject (),
						SIGNAL (availableVariantsChanged (const QStringList&)),
						this,
						SLOT (invalidateClientsIconCache ()));
				connect (clEntry->GetObject (),
						SIGNAL (gotMessage (QObject*)),
						this,
						SLOT (handleEntryGotMessage (QObject*)));
				connect (clEntry->GetObject (),
						SIGNAL (nameChanged (const QString&)),
						this,
						SLOT (handleEntryNameChanged (const QString&)));
				connect (clEntry->GetObject (),
						SIGNAL (avatarChanged (const QImage&)),
						this,
						SLOT (invalidateSmoothAvatarCache ()));
				connect (clEntry->GetObject (),
						SIGNAL (avatarChanged (const QImage&)),
						this,
						SLOT (updateItem ()));

				const QByteArray& id = clEntry->GetEntryID ();
				ID2Entry_ [id] = clEntry->GetObject ();

				const QStringList& groups =
					clEntry->GetEntryType () == Plugins::ICLEntry::ETUnauthEntry ?
							QStringList (tr ("Unauthorized users")) :
							clEntry->Groups ();
				QList<QStandardItem*> catItems =
						GetCategoriesItems (groups, accItem);
				Q_FOREACH (QStandardItem *catItem, catItems)
				{
					QStandardItem *clItem = new QStandardItem (clEntry->GetEntryName ());
					clItem->setEditable (false);
					QObject *accObj = clEntry->GetParentAccount ();
					clItem->setData (QVariant::fromValue<QObject*> (accObj),
							CLRAccountObject);
					clItem->setData (QVariant::fromValue<QObject*> (clEntry->GetObject ()),
							CLREntryObject);
					clItem->setData (QVariant::fromValue<CLEntryType> (CLETContact),
							CLREntryType);
					catItem->appendRow (clItem);

					Entry2Items_ [clEntry] << clItem;
				}

				HandleStatusChanged (clEntry->GetStatus (), clEntry, QString ());

				ChatTabsManager_->UpdateEntryMapping (id, clEntry->GetObject ());
				ChatTabsManager_->SetChatEnabled (id, true);
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

			QStandardItem* Core::GetAccountItem (const QObject *accountObj)
			{
				for (int i = 0, size = CLModel_->rowCount ();
						i < size; ++i)
					if (CLModel_->item (i)->
								data (CLRAccountObject).value<QObject*> () ==
							accountObj)
						return CLModel_->item (i);
				return 0;
			}

			QStandardItem* Core::GetAccountItem (const QObject *accountObj,
					QMap<const QObject*, QStandardItem*>& accountItemCache)
			{
				if (accountItemCache.contains (accountObj))
					return accountItemCache [accountObj];
				else
				{
					QStandardItem *accountItem = GetAccountItem (accountObj);
					if (accountItem)
						accountItemCache [accountObj] = accountItem;
					return accountItem;
				}
			}

			void Core::HandleStatusChanged (const Plugins::EntryStatus& status,
					Plugins::ICLEntry *entry, const QString& variant)
			{
				invalidateClientsIconCache (entry);
				QString tip = QString ("%1 (%2)<hr />%3 (%4)<hr />%5")
						.arg (entry->GetEntryName ())
						.arg (entry->GetHumanReadableID ())
						.arg (PluginProxyObject_->StateToString (entry->GetStatus ().State_))
						.arg (entry->GetStatus ().StatusString_)
						.arg (tr ("In groups: ") + entry->Groups ().join ("; "));
				Q_FOREACH (const QString& variant, entry->Variants ())
				{
					if (variant.isEmpty ())
						continue;

					tip += QString ("<hr /><strong>%1</strong>: %2 (%3)")
							.arg (variant)
							.arg (PluginProxyObject_->StateToString (entry->GetStatus (variant).State_))
							.arg (entry->GetStatus (variant).StatusString_);
				}

				bool isPrimary = variant.isNull () ||
						entry->Variants ().first () == variant;

				Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
				{
					item->setToolTip (tip);
					if (isPrimary ||
							status.State_ == Plugins::SOffline)
						item->setIcon (GetIconForState (status.State_));
				}
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

				QString filename = XmlSettingsManager::Instance ()
						.property ("StatusIcons").toString ();
				filename += '/';
				filename += iconName;
				QStringList variants;
				variants << filename + ".svg"
						<< filename + ".png"
						<< filename + ".jpg";

				QString path = StatusIconLoader_->GetPath (variants);
				return QIcon (path);
			}

			QMap<QString, QIcon> Core::GetClientIconForEntry (Plugins::ICLEntry *entry)
			{
				if (EntryClientIconCache_.contains (entry))
					return EntryClientIconCache_ [entry];

				QMap<QString, QIcon> result;

				Q_FOREACH (const QString& variant, entry->Variants ())
				{
					QString filename = "default/";
					filename += entry->GetClientInfo (variant) ["client_type"].toString ();
					QStringList variants;
					variants << filename + ".svg"
							<< filename + ".png"
							<< filename + ".jpg";
					result [variant] = QIcon (ClientIconLoader_->GetPath (variants));
				}

				EntryClientIconCache_ [entry] = result;
				return result;
			}

			QImage Core::GetAvatar (Plugins::ICLEntry* entry, int size)
			{
				if (Entry2SmoothAvatarCache_.contains (entry) &&
						(Entry2SmoothAvatarCache_ [entry].width () == size ||
						 Entry2SmoothAvatarCache_ [entry].height () == size))
					return Entry2SmoothAvatarCache_ [entry];

				const QImage& avatar = entry->GetAvatar ();
				if (avatar.isNull () || !avatar.width ())
					return avatar;

				const QImage& scaled = avatar.scaled (size, size,
						Qt::KeepAspectRatio, Qt::SmoothTransformation);
				Entry2SmoothAvatarCache_ [entry] = scaled;
				return scaled;
			}

			QList<QAction*> Core::GetEntryActions (Plugins::ICLEntry *entry)
			{
				if (!Entry2Actions_.contains (entry))
					CreateActionsForEntry (entry);
				UpdateActionsForEntry (entry);

				const QHash<QByteArray, QAction*>& id2action = Entry2Actions_ [entry];
				QList<QAction*> result;
				result << id2action.value ("openchat");
				result << id2action.value ("rename");
				result << id2action.value ("remove");
				result << id2action.value ("authorization");
				result << id2action.value ("kick");
				result << id2action.value ("ban");
				result << id2action.value ("sep_afterban");
				result << id2action.value ("changerole");
				result << id2action.value ("changeaffiliation");
				result << id2action.value ("sep_afterroles");
				result << id2action.value ("vcard");
				result << id2action.value ("leave");
				result << id2action.value ("authorize");
				result << id2action.value ("denyauth");
				result << entry->GetActions ();
				result.removeAll (0);
				return result;
			}

			QList<Core::CLEntryActionArea> Core::GetAreasForAction (const QAction *action) const
			{
				return Action2Areas_.value (action,
						QList<CLEntryActionArea> () << CLEAAContactListCtxtMenu);
			}

			void Core::RecalculateUnreadForParents (QStandardItem *clItem)
			{
				QStandardItem *category = clItem->parent ();
				int sum = 0;
				for (int i = 0, rc = category->rowCount ();
						i < rc; ++i)
					sum += category->child (i)->data (CLRUnreadMsgCount).toInt ();
				category->setData (sum, CLRUnreadMsgCount);
			}

			void Core::CreateActionsForEntry (Plugins::ICLEntry *entry)
			{
				if (Entry2Actions_.contains (entry))
					Q_FOREACH (const QAction *action,
								Entry2Actions_.take (entry).values ())
					{
						Action2Areas_.remove (action);
						delete action;
					}

				QAction *openChat = new QAction (tr ("Open chat"), entry->GetObject ());
				openChat->setProperty ("ActionIcon", "azoth_openchat");
				connect (openChat,
						SIGNAL (triggered ()),
						this,
						SLOT (handleActionOpenChatTriggered ()));
				Entry2Actions_ [entry] ["openchat"] = openChat;
				Action2Areas_ [openChat] << CLEAAContactListCtxtMenu;

				if (entry->GetEntryFeatures () & Plugins::ICLEntry::FSupportsRenames)
				{
					QAction *rename = new QAction (tr ("Rename"), entry->GetObject ());
					connect (rename,
							SIGNAL (triggered ()),
							this,
							SLOT (handleActionRenameTriggered ()));
					rename->setProperty ("ActionIcon", "azoth_rename");
					Entry2Actions_ [entry] ["rename"] = rename;
					Action2Areas_ [rename] << CLEAAContactListCtxtMenu;
				}

				if (entry->GetEntryFeatures () & Plugins::ICLEntry::FSupportsAuth)
				{
					QMenu *authMenu = new QMenu (tr ("Authorization"));
					authMenu->menuAction ()->setProperty ("ActionIcon", "azoth_menu_authorization");
					Entry2Actions_ [entry] ["authorization"] = authMenu->menuAction ();
					Action2Areas_ [authMenu->menuAction ()] << CLEAAContactListCtxtMenu;

					QAction *revokeAuth = authMenu->addAction (tr ("Revoke"),
							this, SLOT (handleActionRevokeAuthTriggered ()));
					revokeAuth->setProperty ("ActionIcon", "azoth_auth_revoke");
					revokeAuth->setProperty ("Azoth/WithReason", false);

					QAction *revokeAuthReason = authMenu->addAction (tr ("Revoke with reason..."),
							this, SLOT (handleActionRevokeAuthTriggered ()));
					revokeAuthReason->setProperty ("ActionIcon", "azoth_auth_revoke_reason");
					revokeAuthReason->setProperty ("Azoth/WithReason", true);

					QAction *unsubscribe = authMenu->addAction (tr ("Unsubscribe"),
							this, SLOT (handleActionUnsubscribeTriggered ()));
					unsubscribe->setProperty ("ActionIcon", "azoth_auth_unsubscribe");
					unsubscribe->setProperty ("Azoth/WithReason", false);

					QAction *unsubscribeReason = authMenu->addAction (tr ("Unsubscribe with reason..."),
							this, SLOT (handleActionUnsubscribeTriggered ()));
					unsubscribeReason->setProperty ("ActionIcon", "azoth_auth_unsubscribe");
					unsubscribeReason->setProperty ("Azoth/WithReason", true);

					QAction *rerequest = authMenu->addAction (tr ("Rerequest authentication"),
							this, SLOT (handleActionRerequestTriggered ()));
					rerequest->setProperty ("ActionIcon", "azoth_auth_rerequest");
					rerequest->setProperty ("Azoth/WithReason", false);

					QAction *rerequestReason = authMenu->addAction (tr ("Rerequest authentication with reason.."),
							this, SLOT (handleActionRerequestTriggered ()));
					rerequestReason->setProperty ("ActionIcon", "azoth_auth_rerequest");
					rerequestReason->setProperty ("Azoth/WithReason", true);
				}

				if (entry->GetEntryType () != Plugins::ICLEntry::ETMUC)
				{
					QAction *vcard = new QAction (tr ("VCard"), entry->GetObject ());
					connect (vcard,
							SIGNAL (triggered ()),
							this,
							SLOT (handleActionVCardTriggered ()));
					vcard->setProperty ("ActionIcon", "azoth_vcard");
					Entry2Actions_ [entry] ["vcard"] = vcard;
					Action2Areas_ [vcard] << CLEAAContactListCtxtMenu;
				}

				if (entry->GetEntryType () == Plugins::ICLEntry::ETPrivateChat)
				{
					QAction *kick = new QAction (tr ("Kick"), entry->GetObject ());
					connect (kick,
							SIGNAL (triggered ()),
							this,
							SLOT (handleActionRoleTriggered ()));
					kick->setProperty ("ActionIcon", "azoth_kick");
					kick->setProperty ("Azoth/TargetRole",
							QVariant::fromValue<Plugins::IMUCEntry::MUCRole> (Plugins::IMUCEntry::MUCRNone));
					Entry2Actions_ [entry] ["kick"] = kick;
					Action2Areas_ [kick] << CLEAAContactListCtxtMenu;

					QAction *ban = new QAction (tr ("Ban"), entry->GetObject ());
					connect (ban,
							SIGNAL (triggered ()),
							this,
							SLOT (handleActionAffTriggered ()));
					ban->setProperty ("ActionIcon", "azoth_ban");
					ban->setProperty ("Azoth/TargetAffiliation",
							QVariant::fromValue<Plugins::IMUCEntry::MUCAffiliation> (Plugins::IMUCEntry::MUCAOutcast));
					Entry2Actions_ [entry] ["ban"] = ban;
					Action2Areas_ [ban] << CLEAAContactListCtxtMenu;

					QAction *sep = Util::CreateSeparator (entry->GetObject ());
					Entry2Actions_ [entry] ["sep_afterban"] = sep;
					Action2Areas_ [sep] << CLEAAContactListCtxtMenu;

					QMenu *changeRole = new QMenu (tr ("Change role"));
					changeRole->menuAction ()->setProperty ("ActionIcon", "azoth_menu_changerole");
					Entry2Actions_ [entry] ["changerole"] = changeRole->menuAction ();
					Action2Areas_ [changeRole->menuAction ()] << CLEAAContactListCtxtMenu;

					QAction *visitorRole = changeRole->addAction (tr ("Visitor"),
							this, SLOT (handleActionRoleTriggered ()));
					visitorRole->setProperty ("ActionIcon", "azoth_role_visitor");
					visitorRole->setParent (entry->GetObject ());
					visitorRole->setCheckable (true);
					visitorRole->setProperty ("Azoth/TargetRole",
							QVariant::fromValue<Plugins::IMUCEntry::MUCRole> (Plugins::IMUCEntry::MUCRVisitor));

					QAction *participantRole = changeRole->addAction (tr ("Participant"),
							this, SLOT (handleActionRoleTriggered ()));
					participantRole->setProperty ("ActionIcon", "azoth_role_participant");
					participantRole->setParent (entry->GetObject ());
					participantRole->setCheckable (true);
					participantRole->setProperty ("Azoth/TargetRole",
							QVariant::fromValue<Plugins::IMUCEntry::MUCRole> (Plugins::IMUCEntry::MUCRParticipant));

					QAction *moderatorRole = changeRole->addAction (tr ("Moderator"),
							this, SLOT (handleActionRoleTriggered ()));
					moderatorRole->setProperty ("ActionIcon", "azoth_role_moderator");
					moderatorRole->setParent (entry->GetObject ());
					moderatorRole->setCheckable (true);
					moderatorRole->setProperty ("Azoth/TargetRole",
							QVariant::fromValue<Plugins::IMUCEntry::MUCRole> (Plugins::IMUCEntry::MUCRModerator));

					QMenu *changeAff = new QMenu (tr ("Change affiliation"));
					changeAff->menuAction ()->setProperty ("ActionIcon", "azoth_menu_changeaffiliation");
					Entry2Actions_ [entry] ["changeaffiliation"] = changeAff->menuAction ();
					Action2Areas_ [changeAff->menuAction ()] << CLEAAContactListCtxtMenu;

					QAction *noneAff = changeAff->addAction (tr ("None"),
							this, SLOT (handleActionAffTriggered ()));
					noneAff->setProperty ("ActionIcon", "azoth_affiliation_none");
					noneAff->setParent (entry->GetObject ());
					noneAff->setCheckable (true);
					noneAff->setProperty ("Azoth/TargetAffiliation",
							QVariant::fromValue<Plugins::IMUCEntry::MUCAffiliation> (Plugins::IMUCEntry::MUCANone));

					QAction *memberAff = changeAff->addAction (tr ("Member"),
							this, SLOT (handleActionAffTriggered ()));
					memberAff->setProperty ("ActionIcon", "azoth_affiliation_member");
					memberAff->setParent (entry->GetObject ());
					memberAff->setCheckable (true);
					memberAff->setProperty ("Azoth/TargetAffiliation",
							QVariant::fromValue<Plugins::IMUCEntry::MUCAffiliation> (Plugins::IMUCEntry::MUCAMember));

					QAction *adminAff = changeAff->addAction (tr ("Admin"),
							this, SLOT (handleActionAffTriggered ()));
					adminAff->setProperty ("ActionIcon", "azoth_affiliation_admin");
					adminAff->setParent (entry->GetObject ());
					adminAff->setCheckable (true);
					adminAff->setProperty ("Azoth/TargetAffiliation",
							QVariant::fromValue<Plugins::IMUCEntry::MUCAffiliation> (Plugins::IMUCEntry::MUCAAdmin));

					QAction *ownerAff = changeAff->addAction (tr ("Owner"),
							this, SLOT (handleActionAffTriggered ()));
					ownerAff->setProperty ("ActionIcon", "azoth_affiliation_owner");
					ownerAff->setParent (entry->GetObject ());
					ownerAff->setCheckable (true);
					ownerAff->setProperty ("Azoth/TargetAffiliation",
							QVariant::fromValue<Plugins::IMUCEntry::MUCAffiliation> (Plugins::IMUCEntry::MUCAOwner));

					sep = Util::CreateSeparator (entry->GetObject ());
					Entry2Actions_ [entry] ["sep_afterroles"] = sep;
					Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
				}
				else if (entry->GetEntryType () == Plugins::ICLEntry::ETMUC)
				{
					QAction *leave = new QAction (tr ("Leave"), entry->GetObject ());
					leave->setProperty ("ActionIcon", "azoth_leave");
					connect (leave,
							SIGNAL (triggered ()),
							this,
							SLOT (handleActionLeaveTriggered ()));
					Entry2Actions_ [entry] ["leave"] = leave;
					Action2Areas_ [leave] << CLEAAContactListCtxtMenu
							<< CLEAATabCtxtMenu;
				}
				else if (entry->GetEntryType () == Plugins::ICLEntry::ETUnauthEntry)
				{
					QAction *authorize = new QAction (tr ("Authorize"), entry->GetObject ());
					authorize->setProperty ("ActionIcon", "azoth_authorize");
					connect (authorize,
							SIGNAL (triggered ()),
							this,
							SLOT (handleActionAuthorizeTriggered ()));
					Entry2Actions_ [entry] ["authorize"] = authorize;
					Action2Areas_ [authorize] << CLEAAContactListCtxtMenu;

					QAction *denyAuth = new QAction (tr ("Deny authorization"), entry->GetObject ());
					denyAuth->setProperty ("ActionIcon", "azoth_denyauth");
					connect (denyAuth,
							 SIGNAL (triggered ()),
							 this,
							 SLOT (handleActionDenyAuthTriggered ()));
					Entry2Actions_ [entry] ["denyauth"] = denyAuth;
					Action2Areas_ [denyAuth] << CLEAAContactListCtxtMenu;
				}
				else if (entry->GetEntryType () == Plugins::ICLEntry::ETChat)
				{
					QAction *remove = new QAction (tr ("Remove"), entry->GetObject ());
					remove->setProperty ("ActionIcon", "remove");
					connect (remove,
							SIGNAL (triggered ()),
							this,
							SLOT (handleActionRemoveTriggered ()));
					Entry2Actions_ [entry] ["remove"] = remove;
					Action2Areas_ [remove] << CLEAAContactListCtxtMenu;
				}

				struct Entrifier
				{
					QVariant Entry_;

					Entrifier (const QVariant& entry)
					: Entry_ (entry)
					{
					}

					void Do (const QList<QAction*>& actions)
					{
						Q_FOREACH (QAction *act, actions)
						{
							act->setProperty ("Azoth/Entry", Entry_);
							act->setParent (Entry_.value<Plugins::ICLEntry*> ()->GetObject ());
							QMenu *menu = act->menu ();
							if (menu)
								Do (menu->actions ());
						}
					}
				} entrifier (QVariant::fromValue<Plugins::ICLEntry*> (entry));
				entrifier.Do (Entry2Actions_ [entry].values ());
			}

			void Core::UpdateActionsForEntry (Plugins::ICLEntry *entry)
			{
				Plugins::IAccount *account = qobject_cast<Plugins::IAccount*> (entry->GetParentAccount ());
				const bool isOnline = account->GetState ().State_ != Plugins::SOffline;
				if (entry->GetEntryType () != Plugins::ICLEntry::ETMUC)
				{
					bool enableVCard =
							account->GetAccountFeatures () & Plugins::IAccount::FCanViewContactsInfoInOffline ||
							isOnline;
					Entry2Actions_ [entry] ["vcard"]->setEnabled (enableVCard);
				}

				if (entry->GetEntryType () == Plugins::ICLEntry::ETChat)
				{
					Entry2Actions_ [entry] ["remove"]->setEnabled (isOnline);
					if (Entry2Actions_ [entry] ["authorization"])
						Entry2Actions_ [entry] ["authorization"]->setEnabled (isOnline);
				}

				Plugins::IMUCEntry *mucEntry =
						qobject_cast<Plugins::IMUCEntry*> (entry->GetParentCLEntry ());
				if (entry->GetEntryType () == Plugins::ICLEntry::ETPrivateChat &&
						!mucEntry)
					qWarning () << Q_FUNC_INFO
							<< "parent of"
							<< entry->GetObject ()
							<< entry->GetParentCLEntry ()
							<< "doesn't implement IMUCEntry";

				if (entry->GetEntryType () == Plugins::ICLEntry::ETPrivateChat &&
						mucEntry)
				{
					QList<QAction*> changeRoleActions;
					changeRoleActions << Entry2Actions_ [entry] ["kick"];
					changeRoleActions << Entry2Actions_ [entry] ["changerole"]->menu ()->actions ();
					Q_FOREACH (QAction *act, changeRoleActions)
					{
						Plugins::IMUCEntry::MUCRole target =
								act->property ("Azoth/TargetRole").value<Plugins::IMUCEntry::MUCRole> ();
						act->setEnabled (mucEntry->MayChangeRole (entry->GetObject (), target));
						act->setChecked (mucEntry->GetRole (entry->GetObject ()) == target);
					}

					QList<QAction*> changeAffActions;
					changeAffActions << Entry2Actions_ [entry] ["ban"];
					changeAffActions << Entry2Actions_ [entry] ["changeaffiliation"]->menu ()->actions ();

					Q_FOREACH (QAction *act, changeAffActions)
					{
						Plugins::IMUCEntry::MUCAffiliation target =
								act->property ("Azoth/TargetAffiliation").value<Plugins::IMUCEntry::MUCAffiliation> ();
						act->setEnabled (mucEntry->MayChangeAffiliation (entry->GetObject (), target));
						act->setChecked (mucEntry->GetAffiliation (entry->GetObject ()) == target);
					}
				}
			}

			QString Core::GetReason (const QString& id, const QString& text)
			{
				return QInputDialog::getText (0,
							tr ("Enter reason"),
							text);
			}

			void Core::ManipulateAuth (const QString& id, const QString& text,
					boost::function<void (Plugins::IAuthable*, const QString&)> func)
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();
				Plugins::IAuthable *authable =
						qobject_cast<Plugins::IAuthable*> (entry->GetObject ());
				if (!authable)
				{
					qWarning () << Q_FUNC_INFO
							<< entry->GetObject ()
							<< "doesn't implement IAuthable";
					return;
				}

				QString reason;
				if (action->property ("Azoth/WithReason").toBool ())
				{
					reason = GetReason (id, text.arg (entry->GetEntryName ()));
					if (reason.isEmpty ())
						return;
				}
				func (authable, reason);
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

			void Core::handleMucJoinRequested ()
			{
				QList<Plugins::IAccount*> accounts;
				Q_FOREACH (QObject *protoPlugin, ProtocolPlugins_)
				{
					QObjectList protocols =
							qobject_cast<Plugins::IProtocolPlugin*> (protoPlugin)->GetProtocols ();
					Q_FOREACH (QObject *protoObj, protocols)
					{
						Plugins::IProtocol *proto = qobject_cast<Plugins::IProtocol*> (protoObj);
						if (!(proto->GetFeatures () & Plugins::IProtocol::PFMUCsJoinable))
							continue;

						QObjectList accountObjs = proto->GetRegisteredAccounts ();
						Q_FOREACH (QObject *accountObj, accountObjs)
							accounts << qobject_cast<Plugins::IAccount*> (accountObj);
					}
				}

				JoinConferenceDialog *dia = new JoinConferenceDialog (accounts, Proxy_->GetMainWindow ());
				dia->show ();
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

				emit accountAdded (account);

				QStandardItem *accItem = new QStandardItem (account->GetAccountName ());
				accItem->setData (QVariant::fromValue<QObject*> (accObject),
						CLRAccountObject);
				accItem->setData (QVariant::fromValue<CLEntryType> (CLETAccount),
						CLREntryType);
				accItem->setIcon (GetIconForState (account->GetState ().State_));
				CLModel_->appendRow (accItem);

				accItem->setEditable (false);

				QList<QStandardItem*> clItems;
				Q_FOREACH (QObject *clObj, account->GetCLEntries ())
				{
					Plugins::ICLEntry *clEntry = qobject_cast<Plugins::ICLEntry*> (clObj);
					if (!clEntry)
					{
						qWarning () << Q_FUNC_INFO
								<< "entry doesn't implement ICLEntry"
								<< clObj
								<< account;
						continue;
					}

					AddCLEntry (clEntry, accItem);
				}

				connect (accObject,
						SIGNAL (gotCLItems (const QList<QObject*>&)),
						this,
						SLOT (handleGotCLItems (const QList<QObject*>&)));
				connect (accObject,
						SIGNAL (removedCLItems (const QList<QObject*>&)),
						this,
						SLOT (handleRemovedCLItems (const QList<QObject*>&)));
				connect (accObject,
						SIGNAL (authorizationRequested (QObject*, const QString&)),
						this,
						SLOT (handleAuthorizationRequested (QObject*, const QString&)));
				connect (accObject,
						SIGNAL (itemSubscribed (QObject*, const QString&)),
						this,
						SLOT (handleItemSubscribed (QObject*, const QString&)));
				connect (accObject,
						SIGNAL (itemUnsubscribed (QObject*, const QString&)),
						this,
						SLOT (handleItemUnsubscribed (QObject*, const QString&)));
				connect (accObject,
						SIGNAL (itemUnsubscribed (const QString&, const QString&)),
						this,
						SLOT (handleItemUnsubscribed (const QString&, const QString&)));
				connect (accObject,
						SIGNAL (statusChanged (const Plugins::EntryStatus&)),
						this,
						SLOT (handleAccountStatusChanged (const Plugins::EntryStatus&)));
			}

			void Core::handleAccountRemoved (QObject *account)
			{
				Plugins::IAccount *accFace =
						qobject_cast<Plugins::IAccount*> (account);
						if (!accFace)
				{
					qWarning () << Q_FUNC_INFO
							<< "account doesn't implement Plugins::IAccount*"
							<< account
							<< sender ();
					return;
				}

				emit accountRemoved (accFace);

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

					if (Entry2Items_.contains (entry))
						continue;

					QObject *accountObj = entry->GetParentAccount ();
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

					if (entry->GetEntryType () & Plugins::ICLEntry::ETMUC)
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

					ChatTabsManager_->SetChatEnabled (entry->GetEntryID (), false);

					Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
					{
						QStandardItem *category = item->parent ();
						QString text = category->text ();
						category->removeRow (item->row ());

						if (!category->rowCount ())
						{
							QStandardItem *account = category->parent ();
							account->removeRow (category->row ());
							Account2Category2Item_ [account].remove (text);
						}
					}
					Entry2Items_.remove (entry);

					Entry2Actions_.remove (entry);

					ID2Entry_.remove (entry->GetEntryID ());

					invalidateClientsIconCache (clitem);
				}
			}

			void Core::handleAccountStatusChanged (const Plugins::EntryStatus& status)
			{
				for (int i = 0, size = CLModel_->rowCount (); i < size; ++i)
				{
					QStandardItem *item = CLModel_->item (i);
					if (item->data (CLRAccountObject).value<QObject*> () != sender ())
						continue;

					item->setIcon (GetIconForState (status.State_));
					return;
				}

				qWarning () << Q_FUNC_INFO
						<< "item for account"
						<< sender ()
						<< "not found";
			}

			void Core::handleStatusChanged (const Plugins::EntryStatus& status, const QString& variant)
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (sender ());
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender is not a ICLEntry:"
							<< sender ();
					return;
				}

				HandleStatusChanged (status, entry, variant);
			}

			void Core::handleEntryNameChanged (const QString& newName)
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (sender ());
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender is not a ICLEntry:"
							<< sender ();
					return;
				}

				Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
					item->setText (newName);
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

				Plugins::ICLEntry *other = qobject_cast<Plugins::ICLEntry*> (msg->OtherPart ());
				if (!other && msg->OtherPart ())
				{
					qWarning () << Q_FUNC_INFO
							<< "message's other part cannot be cast to ICLEntry"
							<< msg->OtherPart ();
					return;
				}

				Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
				emit hookGotMessage (proxy, msgObj);
				if (proxy->IsCancelled ())
					return;

				if (msg->GetMessageType () != Plugins::IMessage::MTMUCMessage &&
						msg->GetMessageType () != Plugins::IMessage::MTChatMessage)
					return;

				Plugins::ICLEntry *parentCL = qobject_cast<Plugins::ICLEntry*> (msg->ParentCLEntry ());

				if (ShouldCountUnread (parentCL, msg))
					Q_FOREACH (QStandardItem *item, Entry2Items_ [parentCL])
					{
						int prevValue = item->data (CLRUnreadMsgCount).toInt ();
						item->setData (prevValue + 1, CLRUnreadMsgCount);
						RecalculateUnreadForParents (item);
					}

				if (msg->GetDirection () == Plugins::IMessage::DIn &&
						!ChatTabsManager_->IsActiveChat (parentCL))
				{
					QString msgString;
					switch (msg->GetMessageType ())
					{
					case Plugins::IMessage::MTChatMessage:
						msgString = tr ("Incoming chat message from <em>%1</em>.")
								.arg (other->GetEntryName ());
						break;
					case Plugins::IMessage::MTMUCMessage:
					{
						if (IsHighlightMessage (msg))
							msgString = tr ("Highlighted in conference <em>%1</em> by <em>%2</em>.")
									.arg (parentCL->GetEntryName ())
									.arg (other->GetEntryName ());
						break;
					}
					default:
						break;
					}

					if (msgString.size ())
					{
						Entity e = Util::MakeNotification ("Azoth",
								msgString,
								PInfo_);
						emit gotEntity (e);
					}
				}
			}

			void Core::handleAuthorizationRequested (QObject *entryObj, const QString& msg)
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (entryObj);
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< entryObj
							<< "doesn't implement ICLEntry";
					return;
				}

				QStandardItem *accItem = GetAccountItem (sender ());
				if (!accItem)
				{
					qWarning () << Q_FUNC_INFO
							<< "no account item for"
							<< sender ();
					return;
				}

				AddCLEntry (entry, accItem);

				QString str = msg.isEmpty () ?
						tr ("Subscription requested by %1.")
							.arg (entry->GetEntryName ()) :
						tr ("Subscription requested by %1: %2.")
							.arg (entry->GetEntryName ())
							.arg (msg);
				emit gotEntity (Util::MakeNotification ("Azoth", str, PInfo_));
			}

			/** @todo Option for disabling notifications of subscription events.
			 */
			void Core::handleItemSubscribed (QObject *entryObj, const QString& msg)
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (entryObj);
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< entryObj
							<< "doesn't implement ICLEntry";
					return;
				}

				QString str = msg.isEmpty () ?
						tr ("%1 (%2) subscribed to us.")
							.arg (entry->GetEntryName ())
							.arg (entry->GetHumanReadableID ()) :
						tr ("%1 (%2) subscribed to us: %3.")
							.arg (entry->GetEntryName ())
							.arg (entry->GetHumanReadableID ())
							.arg (msg);
				emit gotEntity (Util::MakeNotification ("Azoth", str, PInfo_));
			}

			/** @todo Option for disabling notifications of unsubscription events.
			 */
			void Core::handleItemUnsubscribed (QObject *entryObj, const QString& msg)
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (entryObj);
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< entryObj
							<< "doesn't implement ICLEntry";
					return;
				}

				QString str = msg.isEmpty () ?
						tr ("%1 (%2) unsubscribed from us.")
							.arg (entry->GetEntryName ())
							.arg (entry->GetHumanReadableID ()) :
						tr ("%1 (%2) unsubscribed from us: %3.")
							.arg (entry->GetEntryName ())
							.arg (entry->GetHumanReadableID ())
							.arg (msg);
				emit gotEntity (Util::MakeNotification ("Azoth", str, PInfo_));
			}

			/** @todo Option for disabling notifications of unsubscription events from
			 * non-roster items.
			 */
			void Core::handleItemUnsubscribed (const QString& entryId, const QString& msg)
			{
				QString str = msg.isEmpty () ?
						tr ("%1 unsubscribed from us.")
							.arg (entryId) :
						tr ("%1 unsubscribed from us: %2.")
							.arg (entryId)
							.arg (msg);
				emit gotEntity (Util::MakeNotification ("Azoth", str, PInfo_));
			}

			void Core::updateStatusIconset ()
			{
				QMap<Plugins::State, QIcon> State2IconCache_;
				Q_FOREACH (Plugins::ICLEntry *entry, Entry2Items_.keys ())
				{
					Plugins::State state = entry->GetStatus ().State_;
					if (!State2IconCache_.contains (state))
						State2IconCache_ [state] = GetIconForState (state);

					Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
						item->setIcon (State2IconCache_ [state]);
				}
			}

			void Core::showVCard ()
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (sender ());
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender doesn't implement ICLEntry"
							<< sender ();
					return;
				}

				entry->ShowInfo ();
			}

			void Core::updateItem ()
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (sender ());
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender doesn't implement ICLEntry"
							<< sender ();
					return;
				}

				Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
					item->setText (entry->GetEntryName ());
			}

			void Core::handleClearUnreadMsgCount (QObject *object)
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (object);
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< object
							<< "doesn't implement ICLEntry";
					return;
				}

				Q_FOREACH (QStandardItem *item, Entry2Items_ [entry])
				{
					item->setData (0, CLRUnreadMsgCount);
					RecalculateUnreadForParents (item);
				}
			}

			void Core::invalidateClientsIconCache (QObject *passedObj)
			{
				QObject *obj = passedObj ? passedObj : sender ();
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (obj);
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< obj
							<< "could not be casted to ICLEntry";
					return;
				}

				invalidateClientsIconCache (entry);
			}

			void Core::invalidateClientsIconCache (Plugins::ICLEntry *entry)
			{
				EntryClientIconCache_.remove (entry);
			}

			void Core::invalidateSmoothAvatarCache ()
			{
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (sender ());
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "could not be casted to ICLEntry";
					return;
				}

				Entry2SmoothAvatarCache_.remove (entry);
			}

			void Core::handleActionOpenChatTriggered ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();
				ChatTabsManager_->OpenChat (entry);
			}

			void Core::handleActionRenameTriggered ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();

				const QString& oldName = entry->GetEntryName ();
				const QString& newName = QInputDialog::getText (0,
						tr ("Rename contact"),
						tr ("Please enter new name for the contact %1:")
							.arg (oldName),
						QLineEdit::Normal,
						oldName);

				if (newName.isEmpty () ||
						oldName == newName)
					return;

				entry->SetEntryName (newName);
			}

			void Core::handleActionRemoveTriggered ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();
				Plugins::IAccount *account =
						qobject_cast<Plugins::IAccount*> (entry->GetParentAccount ());
				if (!account)
				{
					qWarning () << Q_FUNC_INFO
							<< entry->GetObject ()
							<< "doesn't return proper IAccount:"
							<< entry->GetParentAccount ();
					return;
				}

				account->RemoveEntry (entry->GetObject ());
			}

			void Core::handleActionRevokeAuthTriggered ()
			{
				ManipulateAuth ("revokeauth",
						tr ("Enter reason for revoking authorization from %1:"),
						&Plugins::IAuthable::RevokeAuth);
			}

			void Core::handleActionUnsubscribeTriggered ()
			{
				ManipulateAuth ("unsubscribe",
						tr ("Enter reason for unsubscribing from %1:"),
						&Plugins::IAuthable::Unsubscribe);
			}

			void Core::handleActionRerequestTriggered ()
			{
				ManipulateAuth ("rerequestauth",
						tr ("Enter reason for rerequesting authorization from %1:"),
						&Plugins::IAuthable::RerequestAuth);
			}

			void Core::handleActionVCardTriggered ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();
				entry->ShowInfo ();
			}

			void Core::handleActionLeaveTriggered ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();
				Plugins::IMUCEntry *mucEntry =
						qobject_cast<Plugins::IMUCEntry*> (entry->GetObject ());
				if (!mucEntry)
				{
					qWarning () << Q_FUNC_INFO
							<< "hm, requested `leave' on an entry"
							<< entry->GetObject ()
							<< "that doesn't implement IMUCEntry"
							<< sender ();
					return;
				}

				mucEntry->Leave ();
			}

			void Core::handleActionAuthorizeTriggered ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();
				Plugins::IAccount *account =
						qobject_cast<Plugins::IAccount*> (entry->GetParentAccount ());
				if (!account)
				{
					qWarning () << Q_FUNC_INFO
							<< "parent account doesn't implement IAccount:"
							<< entry->GetParentAccount ();
					return;
				}
				const QString& id = entry->GetHumanReadableID ();
				account->Authorize (entry->GetObject ());
				account->RequestAuth (id);
			}

			void Core::handleActionDenyAuthTriggered ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();
				Plugins::IAccount *account =
						qobject_cast<Plugins::IAccount*> (entry->GetParentAccount ());
				if (!account)
				{
					qWarning () << Q_FUNC_INFO
							<< "parent account doesn't implement IAccount:"
							<< entry->GetParentAccount ();
					return;
				}
				account->DenyAuth (entry->GetObject ());
			}

			void Core::handleActionRoleTriggered ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				QVariant property = action->property ("Azoth/TargetRole");
				if (!property.canConvert<Plugins::IMUCEntry::MUCRole> ())
				{
					qWarning () << Q_FUNC_INFO
							<< "can't convert"
							<< property
							<< "to MUCRole";
					return;
				}

				Plugins::IMUCEntry::MUCRole role =
						property.value<Plugins::IMUCEntry::MUCRole> ();

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();
				Plugins::IMUCEntry *mucEntry =
						qobject_cast<Plugins::IMUCEntry*> (entry->GetParentCLEntry ());
				if (!mucEntry)
				{
					int idx = metaObject ()->indexOfEnumerator ("MUCRole");
					qWarning () << Q_FUNC_INFO
							<< entry->GetParentCLEntry ()
							<< "doesn't implement IMUCEntry, tried role "
							<< (idx >= 0 ?
									metaObject ()->enumerator (idx).valueToKey (role) :
									"<unknown enum>");
					return;
				}

				mucEntry->SetRole (entry->GetObject (), role);
			}

			void Core::handleActionAffTriggered ()
			{
				QAction *action = qobject_cast<QAction*> (sender ());
				if (!action)
				{
					qWarning () << Q_FUNC_INFO
							<< sender ()
							<< "is not a QAction";
					return;
				}

				QVariant property = action->property ("Azoth/TargetAffiliation");
				if (!property.canConvert<Plugins::IMUCEntry::MUCAffiliation> ())
				{
					qWarning () << Q_FUNC_INFO
							<< "can't convert"
							<< property
							<< "to MUCAffiliation";
					return;
				}

				Plugins::IMUCEntry::MUCAffiliation aff =
						property.value<Plugins::IMUCEntry::MUCAffiliation> ();

				Plugins::ICLEntry *entry = action->
						property ("Azoth/Entry").value<Plugins::ICLEntry*> ();
				Plugins::IMUCEntry *mucEntry =
						qobject_cast<Plugins::IMUCEntry*> (entry->GetParentCLEntry ());
				if (!mucEntry)
				{
					int idx = metaObject ()->indexOfEnumerator ("MUCAffiliation");
					qWarning () << Q_FUNC_INFO
							<< entry->GetParentCLEntry ()
							<< "doesn't implement IMUCEntry, tried role "
							<< (idx >= 0 ?
									metaObject ()->enumerator (idx).valueToKey (aff) :
									"<unknown enum>");
					return;
				}

				mucEntry->SetAffiliation (entry->GetObject (), aff);
			}
		}
	}
}
