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

#ifndef PLUGINS_AZOTH_INTERFACES_IPROTOCOL_H
#define PLUGINS_AZOTH_INTERFACES_IPROTOCOL_H
#include <QFlags>
#include <QMetaType>

class QObject;
class QIcon;

namespace LeechCraft
{
namespace Azoth
{
	class IAccount;
	class IProtocolPlugin;

	/** @brief Represents a protocol.
	 *
	 * IProtocol class represents a single protocol with its own set of
	 * accounts.
	 * 
	 * A protocol may also implement IURIHandler if it supports handling
	 * of various URIs, like xmpp: for XMPP protocol.
	 * 
	 * When user decides to add a new account within this protocol, the
	 * GetAccountRegistrationWidgets() function is called to get the
	 * list of widgets that should be filled out by the user by the
	 * exact values of his account, and if the user accepts the
	 * registration, RegisterAccount() would be called with those
	 * widgets.
	 * 
	 * @sa IURIHandler
	 */
	class IProtocol
	{
	public:
		virtual ~IProtocol () {}

		/** This enum describes the features that may be supported by a
		 * protocol.
		 */
		enum ProtocolFeature
		{
			/** None of these features are supported by the protocol.
			 */
			PFNone = 0x0,

			/** Multiuser chats are possible in this proto.
			 */
			PFSupportsMUCs = 0x01,

			/** One could join MUCs as he wishes.
			 */
			PFMUCsJoinable = 0x02,
			
			/** This protocol supports in-band registration: accounts
			 * could be registered right from the client.
			 */
			PFSupportsInBandRegistration = 0x04
		};

		Q_DECLARE_FLAGS (ProtocolFeatures, ProtocolFeature);
		
		/** This enum describes the options that may be selected by the
		 * user when adding a new account.
		 */
		enum AccountAddOption
		{
			/** No custom options.
			 */
			AAONoOptions = 0,

			/** User has chosen to register a new account (if the
			 * protocol advertises PFSupportsInBandRegistration
			 * feature).
			 */
			AAORegisterNewAccount = 0x01
		};
		
		Q_DECLARE_FLAGS (AccountAddOptions, AccountAddOption);

		/** @brief Returns the protocol object as a QObject.
		 *
		 * @return Protocol object as QObject.
		 */
		virtual QObject* GetObject () = 0;

		/** Returns the list of features supported by this protocol.
		 */
		virtual ProtocolFeatures GetFeatures () const = 0;

		/** @brief Returns the accounts within this protocol.
		 * 
		 * Each object in the returned list must implement IAccount.
		 *
		 * @return The list of accounts of this protocol.
		 */
		virtual QList<QObject*> GetRegisteredAccounts () = 0;

		/** @brief Returns the pointer to the parent protocol plugin
		 * that this protocol belongs to.
		 *
		 * @return The parent protocol plugin of this protocol, which
		 * must implement IProtocolPlugin.
		 */
		virtual QObject* GetParentProtocolPlugin () const = 0;

		/** @brief Returns the human-readable name of this protocol,
		 * like "Jabber" or "ICQ".
		 *
		 * @return Human-readable name of the protocol.
		 */
		virtual QString GetProtocolName () const = 0;
		
		/** @brief Returns the icon of this protocol.
		 * 
		 * @return The icon of the protocol.
		 */
		virtual QIcon GetProtocolIcon () const = 0;

		/** @brief Returns the protocol ID, which must be unique among
		 * all the protocols.
		 *
		 * @return The unique ID of this protocol.
		 */
		virtual QByteArray GetProtocolID () const = 0;

		/** @brief Returns the widgets used for account addition.
		 * 
		 * The widgets from the returned list are shown in the account
		 * addition wizard in the same order they appear in the returned
		 * list.
		 * 
		 * If the user accepts registering the account, the
		 * RegisterAccount() method would be called with the same set of
		 * widgets in the same order as returned from this function.
		 * 
		 * The ownership is transferred to the caller.
		 * 
		 * @param[in] options The options selected by the user to
		 * perform the account addition.
		 * 
		 * @return The widgets to be shown and filled by the user.
		 * 
		 * @sa RegisterAccount()
		 */
		virtual QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions options) = 0;
		
		/** @brief Adds an account with the given name and widgets.
		 * 
		 * The list of widgets is the same (and in the same order) as
		 * returned from the GetAccountRegistrationWidgets() function.
		 * If this method is called, the widgets were shown to the user,
		 * and the user has accepted the account registration, so the
		 * widgets would be filled by the user to contain the required
		 * values. One could use qobject_cast to cast each widget in the
		 * list to the exact widget type and then use the values filled
		 * by the user.
		 * 
		 * If the account is added successfully, the accountAdded()
		 * signal should be emitted, otherwise nothing should be done.
		 * 
		 * @note Since ownership of the widgets is transferred to the
		 * caller, one shouldn't delete widgets in this method, and one
		 * shouldn't store them to use after this function has returned.
		 * 
		 * @param[in] name The name of the account to be registered.
		 * @param[in] widgets The list of widgets returned from the
		 * GetAccountRegistrationWidgets() and filled by the user.
		 * 
		 * @sa GetAccountRegistrationWidgets()
		 */
		virtual void RegisterAccount (const QString& name, const QList<QWidget*>& widgets) = 0;

		/** @brief Returns the widget used to set up the MUC join options.
		 *
		 * The returned widget must implement IMUCJoinWidget.
		 *
		 * The caller takes the ownership of the widget, so each time
		 * a newly constructed widget should be returned, and the plugin
		 * shouldn't delete the widget by itself.
		 * 
		 * @return The widget used for joining MUCs, which must
		 * implement IMUCJoinWidget.
		 *
		 * @sa IMUCJoinWidget
		 */
		virtual QWidget* GetMUCJoinWidget () = 0;
		
		/** @brief Returns the editor widget for the bookmarks of this
		 * protocol.
		 * 
		 * The returned widget must implement the
		 * IMUCBookmarkEditorWidget interface.
		 * 
		 * This function should create a new widget each time it is
		 * called, since the ownership is transferred to the caller and
		 * the widget will be deleted by the caller when appropriate.
		 * 
		 * @sa IMUCBookmarkEditorWidget
		 */
		virtual QWidget* GetMUCBookmarkEditorWidget () = 0;

		/** @brief Removes the given account.
		 *
		 * This function shouldn't ask anything from the user, just
		 * remove the account.
		 *
		 * If the account is not registered, this function should do
		 * nothing.
		 * 
		 * After the removal, the accountRemoved() signal is expected to
		 * be emitted.
		 * 
		 * @param[in] account The account to remove.
		 */
		virtual void RemoveAccount (QObject *account) = 0;

		/** @brief Notifies about new account.
		 * 
		 * This signal should be emitted whenever a new account appears
		 * in this protocol.
		 * 
		 * @note This function is expected to be a signal.
		 * 
		 * @param[out] account The newly added account, which must
		 * implement IAccount.
		 */
		virtual void accountAdded (QObject *account) = 0;
		
		/** @brief Notifies about an account having been removed.
		 * 
		 * This signal should be emitted whenever an already registered
		 * account is removed, for example, as the result of the call to
		 * RemoveAccount().
		 * 
		 * @note This function is expected to be a signal.
		 * 
		 * @param[out] account The just removed account, which must
		 * implement IAccount.
		 */
		virtual void accountRemoved (QObject *account) = 0;
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS (IProtocol::ProtocolFeatures);
}
}

Q_DECLARE_METATYPE (LeechCraft::Azoth::IProtocol*);
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IProtocol,
		"org.Deviant.LeechCraft.Azoth.IProtocol/1.0");

#endif
