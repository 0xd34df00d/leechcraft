/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_INTERFACES_ICLENTRY_H
#define PLUGINS_AZOTH_INTERFACES_ICLENTRY_H
#include <QFlags>
#include <QMetaType>
#include "imessage.h"
#include "azothcommon.h"

class QAction;
class QImage;

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
	class IAccount;
	class IMessage;

	struct EntryStatus
	{
		State State_;
		QString StatusString_;

		EntryStatus ()
		: State_ (SOffline)
		{
		}

		EntryStatus (State state, const QString& str)
		: State_ (state)
		, StatusString_ (str)
		{
		}
	};

	inline bool operator== (const EntryStatus& es1, const EntryStatus& es2)
	{
		return es1.State_ == es2.State_ &&
				es1.StatusString_ == es2.StatusString_;
	}

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
	 * By default, entries are considered to be normal chat entries, and
	 * Azoth core would manage the chat widget (and, consequently, chat
	 * tabs) for such entries itself. Nevertheless, some entries may
	 * want to have custom behavior for their widgets or even have no
	 * widgets at all. In this case, they set FHasCustomChatWidget flag
	 * in GetEntryFeatures(). In this case Azoth core wouldn't care
	 * about their chat widgets at all. Instead, such entry would need
	 * to implement IMultiTabsWidget, for example, and the corresponding
	 * plugin would need to be a IMultiTabs.
	 */
	class ICLEntry
	{
	public:
		virtual ~ICLEntry () {}

		/** Represents the features that may be supported by a contant list entry.
		 */
		enum Feature
		{
			FPermanentEntry			= 0x0000,	//!< FPermanentEntry If this entry is permanent and would appear in the next session too.
			FSessionEntry			= 0x0001,	//!< FSessionEntry If this entry is not permament and is for this session only.
			FMaskLongetivity		= 0x0003,	//!< 0000011
			FSupportsRenames		= 0x0020,	//!< FSupportsRenames This entry supports renaming, so calls to SetEntryName() are not senseless.
			FHasCustomChatWidget	= 0x0040,	//!< FHasCustomChatWidget This entry has a custom chat widget.
			FCanHaveMultiVariants	= 0x0080	//!< FCanHaveMultiVariants This entry may have multiple delivery variants.
		};

		Q_DECLARE_FLAGS (Features, Feature);

		enum EntryType
		{
			ETChat,			//!< ETChat This entry represents a standard chat.
			ETMUC,			//!< ETMUC This entry represents a multi-user chatroom.
			ETPrivateChat,	//!< ETPrivateChat This entry represents a private conversation in a multi-user chatroom.
			ETUnauthEntry	//!< ETUnauthEntry This entry represents an unauthorized user that has requested authorization.
		};

		/** Returns the entry as a QObject.
		 *
		 * @return Contact list entry as QObject.
		 */
		virtual QObject* GetObject () = 0;

		/** Returns the pointer to the parent account that this entry
		 * belongs to.
		 *
		 * @return The parent account of this entry.
		 */
		virtual QObject* GetParentAccount () const = 0;

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

		/** @brief Returns the ID of this entry.
		 *
		 * The ID must be unique among all entries of this protocol and
		 * should not depend on the value returned by GetEntryName()
		 * (the human-readable name).
		 *
		 * The main difference between this and GetHumanReadableID() is
		 * that GetEntryID() is used for distinguishing different items
		 * in the contact list (and there may be several items for one
		 * remote), while GetHumanReadableID() is used to distinguish
		 * different remotes between each other.
		 *
		 * @return The unique and persistent ID of this entry.
		 *
		 * @sa GetHumanReadableID()
		 */
		virtual QByteArray GetEntryID () const = 0;

		/** @brief Returns the human-readable ID of this entry.
		 *
		 * This function is used to obtain the human-readable identifier
		 * of this entry (for example, Jabber ID in case of XMPP), which
		 * may be not so unique as GetEntryID(). For example, if an
		 * entry exists in the roster, but it has also requested auth,
		 * there would be two entries with the same human-readable ID,
		 * but they would still be distinguished by the result of the
		 * GetEntryID() function.
		 *
		 * Various operations like buddy searches (in protocols that
		 * support this feature like Skype or ICQ) are expected to
		 * operate on strings that are among possible return values of
		 * this function. Also, when initiating entry addition, the
		 * entry is expected to be identified by a similar string.
		 *
		 * The default implementation returns GetEntryID() as an unicode
		 * string.
		 *
		 * @return Human-readable persistent ID of this entry.
		 *
		 * @sa GetEntryID()
		 */
		virtual QString GetHumanReadableID () const
		{
			return QString::fromUtf8 (GetEntryID ().constData ());
		}

		/** @brief Returns the list of human-readable names of the
		 * groups that this entry belongs to.
		 *
		 * @return The list of groups of this item.
		 */
		virtual QStringList Groups () const = 0;

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

		/** @brief Creates the message of the given type to the given
		 * variant.
		 *
		 * Variant is a string from the list returned by Variants(). If
		 * a different string is passed, particularly, a null one, the
		 * implementation must choose the best variant itself: for
		 * example, the resource with the highest priority in XMPP.
		 *
		 * No message should be sent as result of CreateMessage().
		 * Instead, one would later call IMessage::Send() on the
		 * returned message.
		 *
		 * @param[in] type The type of the message.
		 * @param[in] variant The variant to send to.
		 * @param[in] body Message boxy.
		 * @return The prepared message.
		 *
		 * @sa Variants()
		 */
		virtual QObject* CreateMessage (IMessage::MessageType type,
				const QString& variant,
				const QString& body) = 0;

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
		virtual QList<QObject*> GetAllMessages () const = 0;

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

		/** @brief Returns the avatar of this item.
		 *
		 * @return The image with the avatar.
		 */
		virtual QImage GetAvatar () const = 0;

		/** @brief Return string with raw information about the entry.
		 *
		 * @return Human-readable string with information about the entry.
		 */
		virtual QString GetRawInfo () const = 0;

		/** @brief Returns the list of actions for the item.
		 *
		 * The list is showed, for example, when user calls the context
		 * menu on the item, or such. You may want to place actions like
		 * "Info", etc, in this list.
		 *
		 * @return The list of actions.
		 */
		virtual QList<QAction*> GetActions () const = 0;

		/** @brief Returns the AuthStatus between our user and this
		 * remote.
		 *
		 * If auth status is not applicable, ASNone should be returned.
		 *
		 * @return AuthStatus or ASNone if not applicable.
		 */
		virtual AuthStatus GetAuthStatus () const = 0;

		/** @brief This signal should be emitted whenever a new message
		 * is received.
		 *
		 * @note This function is expected to be a signal in subclasses.
		 *
		 * @param[out] msg The message that was just received.
		 */
		virtual void gotMessage (QObject *msg) = 0;

		/** @brief This signal should be emitted whenever the status of
		 * a variant in this entry changes.
		 *
		 * @note This function is expected to be a signal in subclasses.
		 *
		 * @param[out] st The new status of this entry.
		 * @param[out] variant What variant is subject to change.
		 */
		virtual void statusChanged (const EntryStatus& st,
				const QString& variant) = 0;

		/** @brief This signal should be emitted whenever the list of
		 * available variants changes.
		 *
		 * @note This function is expected to be a signal in subclasses.
		 *
		 * @param[out] newVars The list of new variants, as
		 * returned by GetVariants().
		 */
		virtual void availableVariantsChanged (const QStringList& newVars) = 0;

		/** @brief This signal should be emitted whenever the avatar of
		 * this item is changed.
		 *
		 * @note This function is expected to be a signal in subclasses.
		 */
		virtual void avatarChanged (const QImage&) = 0;

		/** @brief This signal should be emitted whenever our copy of
		 * raw information is updated.
		 *
		 * @note This function is expected to be a signal in subclesses.
		 */
		virtual void rawinfoChanged (const QString&) = 0;
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS (ICLEntry::Features);
}
}
}
}

Q_DECLARE_METATYPE (LeechCraft::Plugins::Azoth::Plugins::EntryStatus);

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Azoth::Plugins::ICLEntry,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.ICLEntry/1.0");

#endif
