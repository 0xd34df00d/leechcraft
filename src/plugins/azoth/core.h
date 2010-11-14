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

#ifndef PLUGINS_AZOTH_CORE_H
#define PLUGINS_AZOTH_CORE_H
#include <QObject>
#include <QSet>
#include <interfaces/iinfo.h>
#include <interfaces/azothcommon.h>

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				struct EntryStatus;
				class ICLEntry;
			}

			class ChatTabsManager;

			class Core : public QObject
			{
				Q_OBJECT

				ICoreProxy_ptr Proxy_;

				QObjectList ProtocolPlugins_;
				QList<QAction*> AccountCreatorActions_;
				QList<QAction*> MUCJoinActions_;

				QStandardItemModel *CLModel_;
				ChatTabsManager *ChatTabsManager_;

				typedef QHash<QString, QStandardItem*> Category2Item_t;
				typedef QHash<QStandardItem*, Category2Item_t> Account2Category2Item_t;
				Account2Category2Item_t Account2Category2Item_;

				typedef QHash<Plugins::ICLEntry*, QList<QStandardItem*> > Entry2Items_t;
				Entry2Items_t Entry2Items_;

				Core ();
			public:
				enum CLRoles
				{
					CLRAccountObject = Qt::UserRole + 1,
					CLREntryObject,
					CLREntryType
				};

				enum CLEntryType
				{
					/** Self account.
					 */
					CLETAccount,
					/** Category (under self account).
					 */
					CLETCategory,
					/** Remote contact.
					 */
					CLETContact
				};

				static Core& Instance ();

				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;

				QSet<QByteArray> GetExpectedPluginClasses () const;
				void AddPlugin (QObject*);

				const QObjectList& GetProtocolPlugins () const;

				QList<QAction*> GetAccountCreatorActions () const;
				QList<QAction*> GetMUCJoinActions () const;
				QAbstractItemModel* GetCLModel () const;
				ChatTabsManager* GetChatTabsManager () const;
				void SendEntity (const Entity&);

				/** Opens chat with the remote contact identified by
				 * index (which is from GetCLModel() model). If the
				 * index identifies account or category, this function
				 * does nothing.
				 */
				void OpenChat (const QModelIndex& index);
			private:
				void AddProtocolPlugin (QObject*);

				/** Returns the list of category items for the given
				 * account and categories list. Creates the items if
				 * needed. The items returned are child of account item.
				 *
				 * Categories could be, for example, tags/groups in XMPP
				 * client and such.
				 */
				QList<QStandardItem*> GetCategoriesItems (QStringList categories, QStandardItem *account);

				QStandardItem* GetAccountItem (const QObject *accountObj,
						QMap<const QObject*, QStandardItem*>& accountItemCache);

				void HandleStatusChanged (const Plugins::EntryStatus&, Plugins::ICLEntry*);

				QIcon GetIconForState (Plugins::State) const;
			private slots:
				/** Initiates account registration process.
				 */
				void handleAccountCreatorTriggered ();

				/** Initiates MUC join.
				 */
				void handleMucJoinRequested ();

				/** Handles a new account. This account may be both a
				 * new one (added as a result of user's actions) and
				 * already existing one (in case it was just read from
				 * settings).
				 *
				 * account is expected to implement Plugins::IAccount
				 * interface.
				 */
				void addAccount (QObject *account);

				/** Handles newly added contact list items. Each item is
				 * expected to implement Plugins::ICLEntry. This slot
				 * appends the items to already existing ones, so only
				 * really new ones (during the session lifetime) should
				 * be in the items list.
				 */
				void handleGotCLItems (const QList<QObject*>& items);

				void handleStatusChanged (const Plugins::EntryStatus&);
			signals:
				void gotEntity (const LeechCraft::Entity&);

				/** Emitted after some new account creation actions have
				 * been received from a plugin and thus should be shown
				 * in the UI.
				 */
				void accountCreatorActionsAdded (const QList<QAction*>&);

				/** Emitted after new actions for joining MultiUser
				 * Chatrooms have been added.
				 */
				void mucJoinActionsAdded (const QList<QAction*>&);
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Azoth::Core::CLEntryType);

#endif

