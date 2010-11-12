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

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			class Core : public QObject
			{
				Q_OBJECT

				ICoreProxy_ptr Proxy_;

				QObjectList ProtocolPlugins_;
				QList<QAction*> AccountCreatorActions_;

				QStandardItemModel *CLModel_;

				typedef QMap<QString, QStandardItem*> Category2Item_t;
				typedef QMap<QStandardItem*, Category2Item_t> Account2Category2Item_t;
				Account2Category2Item_t Account2Category2Item_;

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
				QAbstractItemModel* GetCLModel () const;
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
			private slots:
				/** Initiates account registration process.
				 */
				void handleAccountCreatorTriggered ();

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
			signals:
				void gotEntity (const LeechCraft::Entity&);

				/** Emitted after some new account creation actions have
				 * been received from a plugin and thus should be shown
				 * in the UI.
				 */
				void accountCreatorActionsAdded (const QList<QAction*>&);
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Azoth::Core::CLEntryType);

#endif

