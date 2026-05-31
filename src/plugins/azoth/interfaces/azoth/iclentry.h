/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHashFunctions>
#include <QFlags>
#include <QMetaType>
#include <util/azoth/emitters/clentry.h>
#include "azothcommon.h"
#include "iaccount.h"
#include "message.h"

class QAction;
class QImage;

namespace LC::Azoth
{
	class IAccount;
	class IMessage;

	/** @brief Represents a single entry in contact list.
	 *
	 * Everything that should go to the contact list should implement
	 * this interface: plain contacts, metacontacts, transports to other
	 * accounts in protocols like XMPP, and such.
	 *
	 * In some protocol an entry can have several destinations, like
	 * resources in XMPP protocol. In this case, GetVariants() should
	 * return the up-to-date list of variants that this entry may have
	 * as destinations.
	 *
	 * This interface provides only more or less basic functionality.
	 * Advanced features, like drawing attention and such, are in
	 * IAdvancedCLEntry.
	 *
	 * If the CL entry can have an avatar, it makes sense to implement the
	 * IHaveAvatars interface.
	 *
	 * @sa IAdvancedCLEntry
	 * @sa IHaveAvatars
	 */
	class ICLEntry
	{
	protected:
		Emitters::CLEntry Emitter_;

		virtual ~ICLEntry () = default;
	public:
		Emitters::CLEntry& GetCLEntryEmitter () { return Emitter_; }

		/** Represents the features that may be supported by a contant
		 * list entry.
		 */
		enum Feature
		{
			FPermanentEntry			= 0b00000000, ///< The entry is permanent: it belongs to some kind of roster list.
			FSessionEntry			= 0b00000001, ///< The entry is transient, like a chat room participant or a temporary contact.
			FMaskLongetivity		= 0b00000011, ///< Mask for FPermanentEntry and FSessionEntry.
			FSupportsRenames		= 0b00000100, ///< The entry supports renaming, so `SetEntryName()` makes sense.
			FSupportsAuth			= 0b00001000, ///< The entry supports auth manipulations and implements IAuthable.
			FSupportsGrouping		= 0b00010000, ///< The entry supports moving between different groups via `SetGroups()`.
			FSelfContact			= 0b00100000, ///< The entry represents ourselves, like the self-contact in XMPP.
		};

		Q_DECLARE_FLAGS (Features, Feature)

		enum class EntryType : std::uint8_t
		{
			Chat,			///< A standard one-to-one chat.
			MUC,			///< A multi-user chatroom.
			PrivateChat,	///< A chat room member.
			UnauthEntry,	///< An unauthorized user that has requested authorization.
		};

		/** Returns the entry as a QObject.
		 *
		 * @return Contact list entry as QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** Returns the pointer to the parent account that this entry
		 * belongs to.
		 *
		 * @return The parent account of this entry.
		 */
		virtual IAccount* GetParentAccount () const = 0;

		/** Returns the pointer to the parent CL entry, if any.
		 *
		 * This currently only makes sense for private chat entries,
		 * thus private chat entries (those that are of type
		 * ETPrivateChat) should return their parent room CL entry (of
		 * type ETMUC).
		 *
		 * If parent CL entry is not applicable, NULL should be
		 * returned.
		 *
		 * The default implementation returns NULL.
		 *
		 * @return Parent CL entry if applicable, NULL otherwise.
		 */
		virtual ICLEntry* GetParentCLEntry () const
		{
			return nullptr;
		}

		QObject* GetParentCLEntryObject () const
		{
			if (const auto entry = GetParentCLEntry ())
				return entry->GetQObject ();
			return nullptr;
		}

		/** Returns the OR-ed combination of Feature flags that
		 * describes the features supported by this contact list entry.
		 *
		 * @return The features supported by this entry.
		 */
		virtual Features GetEntryFeatures () const = 0;

		/** Returns the type of this entry.
		 *
		 * @return The type of this entry.
		 */
		virtual EntryType GetEntryType () const = 0;

		/** Returns the human-readable name of this entry.
		 *
		 * @return Human-readable name of this entry.
		 *
		 * @sa SetEntryName()
		 */
		virtual QString GetEntryName () const = 0;

		/** @brief Sets the human-readable name of this entry.
		 *
		 * The name is expected to be updated on the server immediately.
		 *
		 * @param[in] name The new human-readable name of this entry.
		 *
		 * @sa GetEntryName()
		 */
		virtual void SetEntryName (const QString& name) = 0;

		[[deprecated("Use GetGlobalPersistentID() or GetGlobalStrongestID()")]]
		QString GetEntryID () const
		{
			return GetGlobalStrongestID ().ToString ();
		}

		[[deprecated("Use GetConventionalID()")]]
		QString GetHumanReadableID () const
		{
			return GetConventionalID ().ToString ();
		}

		/** @brief Returns the global persistent ID of this entry.
		 *
		 * The persistent ID:
		 * - must be unique among all entries belonging to this entry's
		 *   parent account
		 * - must never change during the lifetime of this object
		 * - shall be persistent across restarts: the same logical entry
		 *   shall have the same persistent ID, if it's possible to
		 *   determine at the protocol level
		 *
		 * Not all contexts support persistent IDs, hence the return value
		 * is wrapped in `std::optional`. A good example is anonymous chat
		 * rooms, where the persistent ID is not available, and the
		 * conventional ID depends on the nick.
		 *
		 * @sa GetGlobalPersistentID()
		 * @sa GetConventionalID()
		 */
		virtual std::optional<EntryPersistentId> GetPersistentID () const = 0;

		std::optional<GlobalPersistentId> GetGlobalPersistentID () const
		{
			if (const auto persistentId = GetPersistentID ())
				return GlobalId { GetParentAccount ()->GetAccountID (), *persistentId };
			return {};
		}

		/** @brief Returns the conventional human-readable ID of this entry.
		 *
		 * Unlike GetPersistentID(), the conventional ID is not guaranteed to
		 * be stable throughout the lifetime of the entry,
		 * and it may change over time (in which case
		 * Emitters::CLEntry::conventionalIdChanged() will be emitted).
		 * The canonical example is an entry in an anonymous chat room,
		 * where the conventional ID depends on the nick.
		 *
		 * If it makes sense for the protocol, the conventional ID is also the
		 * ID normally used by humans to exchange contacts (like JID in XMPP,
		 * or phone number in SIP).
		 *
		 * @return Conventional ID of this entry.
		 *
		 * @sa GetPersistentID()
		 */
		virtual EntryConventionalId GetConventionalID () const = 0;

		GlobalConventionalId GetGlobalConventionalID () const
		{
			return GlobalId { GetParentAccount ()->GetAccountID (), GetConventionalID () };
		}

		EntryStrongestId GetStrongestID () const
		{
			if (const auto persistentId = GetPersistentID ())
				return *persistentId;
			return GetConventionalID ();
		}

		GlobalStrongestId GetGlobalStrongestID () const
		{
			if (const auto persistentId = GetGlobalPersistentID ())
				return *persistentId;
			return GetGlobalConventionalID ();
		}

		/** @brief Returns the list of human-readable names of the
		 * groups that this entry belongs to.
		 *
		 * @return The list of groups of this item.
		 */
		virtual QStringList Groups () const = 0;

		/** @brief Sets the list of groups this item belongs to.
		 *
		 * If updating the list of groups is not applicable, this
		 * function should do nothing.
		 *
		 * @param[in] groups The new list of groups.
		 */
		virtual void SetGroups (const QStringList& groups) = 0;

		/** @brief Returns the list of destination variants.
		 *
		 * For example, for an entry representing a contact in XMPP
		 * protocol variants would be the list of resources for the
		 * contact.
		 *
		 * Strings in the list should not be null, though they may be
		 * empty. There should be no duplicates in returned list.
		 *
		 * The strings in the returned list should be sorted in
		 * descending order according to importance. For example, for
		 * XMPP protocol, the first variant should be the resource with
		 * the highest priority.
		 *
		 * @return The list of variants of this entry.
		 */
		virtual QStringList Variants () const = 0;

		/** @brief Sends the message described by `message`.
		 */
		virtual void SendMessage (const OutgoingMessage& message) = 0;

		/** @brief Returns all already sent or received messages.
		 *
		 * Returns the list of all messages sent or received during the
		 * session.
		 *
		 * If the entry represents a MUC, all the messages in the
		 * returned list should have "IN" direction.
		 *
		 * @return The list of messages.
		 */
		virtual QList<IMessage*> GetAllMessages () const = 0;

		/** @brief Purges messages before the given date.
		 *
		 * This function should purge all the messages before the given
		 * date. After the call, the corresponding messages should not
		 * appear in the result of GetAllMessages() function. It's also
		 * suggested to remove them from any internal memory storage to
		 * conserve memory.
		 *
		 * If before is an invalid date, this function should purge all
		 * messages.
		 *
		 * @param[in] before The date before which messages should be
		 * purged.
		 */
		virtual void PurgeMessages (const QDateTime& before) = 0;

		/** @brief Notifies about our chat participation state change.
		 *
		 * If variant is a null string, a variant with the highest
		 * priority should be used.
		 *
		 * @param[in] state Our new chat participation state.
		 * @param[in] variant Target variant.
		 */
		virtual void SetChatPartState (ChatPartState state,
				const QString& variant) = 0;

		/** @brief Returns the current status of a variant of the item.
		 *
		 * Since different variants may have different status, this
		 * function should return the proper status for the given
		 * variant. If no such variant exists, an empty status structure
		 * should be returned.
		 *
		 * @param[in] variant The variant to return status for or null
		 * string for most important variant.
		 * @return The current status.
		 */
		virtual EntryStatus GetStatus (const QString& variant = QString ()) const = 0;

		/** @brief Requests the entry to show dialog with info about it.
		 */
		virtual void ShowInfo () = 0;

		/** @brief Returns the list of actions for the item.
		 *
		 * The list is showed, for example, when user calls the context
		 * menu on the item, or such. You may want to place actions like
		 * "Info", etc, in this list.
		 *
		 * @return The list of actions.
		 */
		virtual QList<QAction*> GetActions () const = 0;

		/** @brief Returns the client information for the given variant.
		 *
		 * The returned map should have the following keys:
		 * - client_type
		 *   The corresponding value is a QString with a client ID.
		 * - client_name
		 *   The corresponding value is a QString with human-readable
		 *   name of the client.
		 * - client_version
		 *   The corresponding value is a QString with human-readable
		 *   version of the client.
		 * - priority
		 *   The corresponding value is a int with the priority of the
		 *   variant. The priority spans from -1024 to 1024, with
		 *   negative values indicating that the message won't be
		 *   delivered to this resource unless it was explicitly created
		 *   to be targeted at this resource.
		 *
		 * @param[in] variant Variant for which to return the client
		 * info.
		 *
		 * @return Human-readable client name of the variant.
		 */
		virtual QMap<QString, QVariant> GetClientInfo (const QString& variant) const = 0;

		/** @brief Called whenever new messages are read.
		 *
		 * This function is called by Azoth Core whenever any unread
		 * messages that could be present in this entry are read. For
		 * example, this may happen when user opens the chat tab with
		 * this entry.
		 *
		 * Please note that this function is always called, even when
		 * there are no messages at all, for example.
		 */
		virtual void MarkMsgsRead () = 0;

		/** @brief Called by Azoth when the chat with the entry is closed.
		 */
		virtual void ChatTabClosed () = 0;
	};
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::Azoth::ICLEntry::Features)
Q_DECLARE_INTERFACE (LC::Azoth::ICLEntry, "org.Deviant.LeechCraft.Azoth.ICLEntry/1.0")
