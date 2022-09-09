/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QSet>
#include <QCache>
#include <QIcon>
#include <QDateTime>
#include <QUrl>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/an/ianemitter.h>
#include <interfaces/iinfo.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/azothcommon.h"
#include "interfaces/azoth/imucentry.h"
#include "interfaces/azoth/iprotocol.h"
#include "interfaces/azoth/isupportriex.h"
#include "sourcetrackingmodel.h"
#include "animatediconmanager.h"

class QStandardItemModel;
class QStandardItem;
class QWebEnginePage;

namespace LC::Util
{
	class ShortcutManager;
	class WkFontsWidget;
}

namespace LC::Azoth
{
	class ICLEntry;
	class IAccount;
	class IMessage;
	class IEmoticonResourceSource;
	class IChatStyleResourceSource;

	class ChatTabsManager;
	class PluginManager;
	class ProxyObject;
	class TransferJobManager;
	class CallManager;
	class ActionsManager;
	class ImportManager;
	class CLModel;
	class ServiceDiscoveryWidget;
	class UnreadQueueManager;
	class CustomStatusesManager;
	class ChatStyleOptionManager;
	class CustomChatStyleManager;
	class CLTooltipManager;
	class CoreCommandsManager;
	class NotificationsManager;
	class AvatarsManager;
	class HistorySyncer;

	struct ChatMsgAppendInfo;

	class Core : public QObject
	{
		Q_OBJECT
		Q_ENUMS (CLRoles CLEntryType CLEntryActionArea)

		ICoreProxy_ptr Proxy_;
		QList<ANFieldData> ANFields_;

		QObjectList ProtocolPlugins_;
		QList<QAction*> AccountCreatorActions_;

		std::shared_ptr<AvatarsManager> AvatarsManager_;
		Util::WkFontsWidget * const FontsWidget_;
		CLTooltipManager * const TooltipManager_;
		CLModel *CLModel_;
		ChatTabsManager *ChatTabsManager_;
		CoreCommandsManager *CoreCommandsManager_;

		typedef QHash<QString, QStandardItem*> Category2Item_t;
		typedef QHash<QStandardItem*, Category2Item_t> Account2Category2Item_t;
		Account2Category2Item_t Account2Category2Item_;

		QHash<IAccount*, EntryStatus> SavedStatus_;

		typedef QHash<ICLEntry*, QList<QStandardItem*>> Entry2Items_t;
		Entry2Items_t Entry2Items_;

		ActionsManager *ActionsManager_;

		typedef QHash<QString, QObject*> ID2Entry_t;
		ID2Entry_t ID2Entry_;

		typedef QCache<ICLEntry*, QImage> Entry2SmoothAvatarCache_t;
		Entry2SmoothAvatarCache_t Entry2SmoothAvatarCache_ { 5 * 1024 * 1024 };

		AnimatedIconManager<QStandardItem*> *ItemIconManager_;

		std::shared_ptr<SourceTrackingModel<IEmoticonResourceSource>> SmilesOptionsModel_;
		std::shared_ptr<SourceTrackingModel<IChatStyleResourceSource>> ChatStylesOptionsModel_;

		std::shared_ptr<PluginManager> PluginManager_;
		std::shared_ptr<ProxyObject> PluginProxyObject_;
		std::shared_ptr<TransferJobManager> XferJobManager_;
		std::shared_ptr<CallManager> CallManager_;
		std::shared_ptr<ImportManager> ImportManager_;
		std::shared_ptr<UnreadQueueManager> UnreadQueueManager_;
		QMap<QByteArray, std::shared_ptr<ChatStyleOptionManager>> StyleOptionManagers_;
		std::shared_ptr<Util::ShortcutManager> ShortcutManager_;
		std::shared_ptr<CustomStatusesManager> CustomStatusesManager_;
		std::shared_ptr<CustomChatStyleManager> CustomChatStyleManager_;
		std::shared_ptr<NotificationsManager> NotificationsManager_;
		std::shared_ptr<HistorySyncer> HistorySyncer_;

		Core ();
	public:
		enum CLRoles
		{
			CLRAccountObject = Qt::UserRole + 1,
			CLREntryObject,
			CLREntryType,
			CLREntryCategory,
			CLRUnreadMsgCount,
			CLRRole,
			CLRAffiliation,
			CLRNumOnline,
			CLRIsMUCCategory
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
		void Release ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
		ProxyObject* GetPluginProxy () const;

		QList<ANFieldData> GetANFields () const;

		QAbstractItemModel* GetSmilesOptionsModel () const;
		IEmoticonResourceSource* GetCurrentEmoSource () const;
		ChatStyleOptionManager* GetChatStylesOptionsManager (const QByteArray&) const;
		Util::ShortcutManager* GetShortcutManager () const;
		CustomStatusesManager* GetCustomStatusesManager () const;
		CustomChatStyleManager* GetCustomChatStyleManager () const;
		UnreadQueueManager* GetUnreadQueueManager () const;
		AvatarsManager* GetAvatarsManager () const;
		Util::WkFontsWidget* GetFontsWidget () const;

		void AddPlugin (QObject*);
		void RegisterHookable (QObject*);

		bool CouldHandle (const Entity&) const;
		void Handle (Entity);

		bool CouldHandleURL (const QUrl&) const;
		void HandleURL (const QUrl&, ICLEntry* = nullptr);
		void HandleURLGeneric (QUrl, bool raise, ICLEntry* = nullptr);

		const QObjectList& GetProtocolPlugins () const;

		QAbstractItemModel* GetCLModel () const;
		ChatTabsManager* GetChatTabsManager () const;
		QList<IAccount*> GetAccounts (std::function<bool (IProtocol*)> = [] (IProtocol*) { return true; }) const;
		QList<IProtocol*> GetProtocols () const;
		IAccount* GetAccount (const QByteArray&) const;

		void UpdateItem (QObject*);

		/** Returns the list of all groups of all chat entries.
		 */
		QStringList GetChatGroups () const;

		void SendEntity (const Entity&);

		/** Returns contact list entry with the given id. The id is the
		 * same as returned by ICLEntry::GetEntryID(). If no such entry
		 * could be found, NULL is returned.
		 */
		QObject* GetEntry (const QString& id) const;

		/** Opens chat with the remote contact identified by index
		 * (which is from GetCLModel() model). If the index identifies
		 * account or category, this function does nothing.
		 */
		void OpenChat (const QModelIndex& index);

		TransferJobManager* GetTransferJobManager () const;

		CallManager* GetCallManager () const;

		SourceTrackingModel<IChatStyleResourceSource>* GetChatStyleSourceModel () const;

		/** Whether the given from the given entry should be counted as
		 * unread message. For example, messages in currently visible
		 * chat session or status messages shouldn't be counted as
		 * unread.
		 */
		bool ShouldCountUnread (const ICLEntry *entry, IMessage *message);

		/** Whether this message should be considered as a the one that
		 * highlights the participant.
		 */
		bool IsHighlightMessage (IMessage*);

		/** @brief Returns the avatar for the given CL entry scaled to
		 * the given size.
		 *
		 * The scale is performed using SmoothTransform and keeping the
		 * aspect ratio.
		 *
		 * @param[in] entry Entry for which to get the avatar.
		 * @return Entry's avatar scaled to the given size.
		 */
		QImage GetAvatar (ICLEntry *entry, int size);

		ActionsManager* GetActionsManager () const;

		CoreCommandsManager* GetCoreCommandsManager () const;

		QString GetSelectedChatTemplate (QObject *entry, QWebEnginePage*) const;
		QUrl GetSelectedChatTemplateURL (QObject*) const;

		bool AppendMessageByTemplate (QWebEnginePage*, QObject*, const ChatMsgAppendInfo&);

		void FrameFocused (QObject*, QWebEnginePage*);

		QString FormatDate (QDateTime, IMessage*);
		QString FormatNickname (QString, IMessage*, const QString& color);
		QString FormatBody (QString body, IMessage *msg, const QList<QColor>& coloring);
		QString HandleSmiles (QString body);

		/** This function increases the number of unread messages by
		 * the given amount, which may be negative.
		 */
		void IncreaseUnreadCount (ICLEntry *entry, int amount = 1);

		int GetUnreadCount (ICLEntry *entry) const;

		IChatStyleResourceSource* GetCurrentChatStyle (QObject*) const;
	private:
		/** Adds the protocol object. The object must implement
		 * IProtocolPlugin interface.
		 *
		 * Creates an entry in the contact list for accounts from the
		 * protocol plugin and creates the actions for adding a new
		 * account in this protocol or joining groupchats.
		 */
		void AddProtocolPlugin (QObject *object);

		/** Adds the resource source object. Currently only smile
		 * resources are supported.
		 */
		void AddResourceSourcePlugin (QObject *object);
		void AddSmileResourceSource (IEmoticonResourceSource*);
		void AddChatStyleResourceSource (IChatStyleResourceSource*);

		/** Adds the given contact list entry to the given account and
		 * performs common initialization tasks.
		 */
		void AddCLEntry (ICLEntry *entry, QStandardItem *accItem);

		/** Returns the list of category items for the given account and
		 * categories list. Creates the items if needed. The returned
		 * items are children of account item.
		 *
		 * Categories could be, for example, tags/groups in XMPP client
		 * and such.
		 */
		QList<QStandardItem*> GetCategoriesItems (QStringList categories, QStandardItem *account);

		/** Returns the QStandardItem for the given account.
		 */
		QStandardItem* GetAccountItem (const IAccount *accountObj);

		/** Returns the QStandardItem for the given account and adds it
		 * into accountItemCache.
		 */
		QStandardItem* GetAccountItem (const IAccount *account,
				QMap<const IAccount*, QStandardItem*>& accountItemCache);

		/** Handles the event of status changes in a contact list entry.
		 */
		void HandleStatusChanged (const EntryStatus& status,
				ICLEntry *entry, const QString& variant);

		/** Checks whether icon representing incoming file should be
		 * drawn for the entry with the given id.
		 */
		void CheckFileIcon (const QString& id);

		/** This functions calculates new value of number of unread
		 * items for the chain of parents of the given item.
		 */
		void RecalculateUnreadForParents (QStandardItem*);

		void RecalculateOnlineForCat (QStandardItem*);

		void HandlePowerNotification (Entity);

		/** Removes one item representing the given CL entry.
		 */
		void RemoveCLItem (QStandardItem*);

		/** Adds the given entry to the given category item.
		 */
		void AddEntryTo (ICLEntry*, QStandardItem*);

		void FillANFields ();
	public slots:
		/** Initiates MUC join by calling the corresponding protocol
		 * plugin's IProtocol::InitiateMUCJoin() function.
		 */
		void handleMucJoinRequested ();

		void handleShowNextUnread ();

		void saveAccountVisibility (IAccount*);
	private slots:
		void handleNewProtocols (const QList<QObject*>&);

		/** Handles a new account. This account may be both a new one
		 * (added as a result of user's actions) and already existing
		 * one (in case it was just read from settings, for example).
		 *
		 * account is expected to implement IAccount interface.
		 */
		void addAccount (QObject *account);

		/** Handles account removal. Basically, just removes it and its
		 * children from the contact list.
		 *
		 * account is expected to implement IAccount interface.
		 */
		void handleAccountRemoved (QObject *account);

		/** Handles newly added contact list items. Each item is
		 * expected to implement ICLEntry. This slot appends the items
		 * to already existing ones, so only really new ones (during the
		 * session lifetime) should be in the items list.
		 */
		void handleGotCLItems (const QList<QObject*>& items);

		/** Handles removal of items previously added to the contact
		 * list. Each item is expected to implement the ICLEntry
		 * interface.
		 *
		 * This slot removes the model items corresponding to the items
		 * removed and also removes those categories that became empty
		 * because of items removal, if any.
		 */
		void handleRemovedCLItems (const QList<QObject*>& items);

		/** Handles the status change of an account to new status.
		 */
		void handleAccountStatusChanged (const EntryStatus& status);

		void handleAccountRenamed (const QString&);

		/** Handles the status change of a CL entry to new status.
		 */
		void handleStatusChanged (const EntryStatus& status, const QString& variant);

		/** Removes the old unneeded variants.
		 */
		void handleVariantsChanged ();

		/** Handles the event of name changes in plugin.
		 */
		void handleEntryNameChanged (const QString& newName);

		/** Handles the event of groups change in plugin.
		 *
		 * If obj is null, the sender() is used, otherwise obj is used.
		 */
		void handleEntryGroupsChanged (QStringList, QObject *obj = 0);

		/** Handles the event of permissions change in entry from plugin.
		 *
		 * If the passed entry is not NULL, it will be used, otherwise
		 * sender() will be used.
		 */
		void handleEntryPermsChanged (ICLEntry *entry = 0);

		/** Handles the message receival from contact list entries.
		 */
		void handleEntryGotMessage (QObject *msg);

		/** Handles nick conflict.
		 */
		void handleNicknameConflict (const QString&);

		/** Handles kicks.
		 */
		void handleBeenKicked (const QString&);

		/** Handles bans.
		 */
		void handleBeenBanned (const QString&);

		/** Is registered in the XmlSettingsManager as handler for
		 * changes of the "StatusIcons" property.
		 */
		void updateStatusIconset ();

		/** Is registered in the XmlSettingsManager as handler for
		 * changes of the "GroupContacts" property.
		 */
		void handleGroupContactsChanged ();

		/** This slot is used to update the model item which is
		 * corresponding to the sender() which is expected to be a
		 * ICLEntry.
		 */
		void updateItem ();

		void handleClearUnreadMsgCount (QObject*);

		void handleGotSDSession (QObject*);

		void handleFileOffered (QObject*);
		void handleJobDeoffered (QObject*);

		void handleRIEXItemsSuggested (QList<LC::Azoth::RIEXItem>, QObject*, QString);
	signals:
		void gotEntity (const LC::Entity&);

		/** Convenient signal for rethrowing the event of an account
		 * being added.
		 */
		void accountAdded (IAccount*);

		/** Convenient signal for rethrowing the event of an account
		 * being removed.
		 */
		void accountRemoved (IAccount*);

		void gotSDWidget (ServiceDiscoveryWidget*);

		// Plugin API
		void hookAddingCLEntryBegin (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookAddingCLEntryEnd (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookEntryStatusChanged (LC::IHookProxy_ptr proxy,
				QObject *entry,
				QString variant);
		void hookFormatDateTime (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QDateTime dateTime,
				QObject *message);
		void hookFormatNickname (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QString nick,
				QObject *message);
		void hookFormatBodyBegin (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookFormatBodyEnd (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookGonnaHandleSmiles (LC::IHookProxy_ptr proxy,
				QString body,
				QString pack);
		void hookGotMessage (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookGotMessage2 (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookIsHighlightMessage (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookShouldCountUnread (LC::IHookProxy_ptr proxy,
				QObject *message);
	};
}

Q_DECLARE_METATYPE (LC::Azoth::Core::CLEntryType)
Q_DECLARE_METATYPE (LC::Azoth::ICLEntry*)
