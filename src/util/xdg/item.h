/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <optional>
#include <QHash>
#include <QDebug>
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include "xdgconfig.h"
#include "itemtypes.h"

namespace LC::Util::XDG
{
	class Item;

	using Item_ptr = std::shared_ptr<Item>;

	/** @brief Describes a single XDG <code>.desktop</code> entry.
	 *
	 * XDG entries can language-dependent fields like name, generic name
	 * or comment. The values of these fields are obtained via GetName(),
	 * GetGenericName() and GetComment() respectively, taking the
	 * language code and returning either the localized field for that
	 * language or the version of the field for the default (typically
	 * English) language.
	 */
	class UTIL_XDG_API Item
	{
		QHash<QString, QString> Name_;
		QHash<QString, QString> GenericName_;
		QHash<QString, QString> Comments_;

		QStringList Categories_;
		QString Command_;
		QString WD_;

		QString IconName_;
		mutable std::optional<QIcon> Icon_;

		bool IsHidden_ = false;
		Type Type_ = Type::Other;
	public:
		/** @brief Checks whether \em left and \em right are equal.
		 *
		 * The icon field obtained via GetIcon() is \em not checked for
		 * equality.
		 *
		 * @param[in] left First XDG item to check for equality.
		 * @param[in] right Second XDG item to check for equality.
		 * @return Whether \em left and \em right are equal.
		 */
		friend UTIL_XDG_API bool operator== (const Item& left, const Item& right);

		/** @brief Checks whether \em left and \em right are not equal.
		 *
		 * The icon field obtained via GetIcon() is \em not checked for
		 * equality.
		 *
		 * @param[in] left First XDG item to check for inequality.
		 * @param[in] right Second XDG item to check for inequality.
		 * @return Whether \em left and \em right are not equal.
		 */
		friend UTIL_XDG_API bool operator!= (const Item& left, const Item& right);

		/** @brief Checks whether this XDG item is valid.
		 *
		 * A valid item has name field set for at least one language.
		 *
		 * @return Whether this XDG item is valid.
		 */
		bool IsValid () const;

		/** @brief Checks whether this XDG item should be hidden.
		 *
		 * A hidden XDG item is not presented to the user in start menus
		 * and so on.
		 *
		 * @return Whether this item is hidden.
		 */
		bool IsHidden () const;

		/** @brief Executes this item, if possible.
		 *
		 * Depending on the type of this item, execution can mean
		 * launching an application (for Type::Application), opening a
		 * default URL handler (for Type::URL) and so on.
		 *
		 * @param[in] proxy The ICoreProxy_ptr object to use if needed
		 * during execution.
		 */
		void Execute (const ICoreProxy_ptr& proxy) const;

		/** @brief Returns the name of this item.
		 *
		 * @param[in] language The code of the desired language for the
		 * localized name.
		 * @return The localized name matching the \em language, or the
		 * default name if there is no localized one for the \em language.
		 *
		 * @sa GetGenericName()
		 */
		QString GetName (const QString& language) const;

		/** @brief Returns the generic name of this item.
		 *
		 * @param[in] language The code of the desired language for the
		 * localized generic name.
		 * @return The localized generic name matching the \em language,
		 * or the default generic name if there is no localized one for
		 * the \em language.
		 *
		 * @sa GetName()
		 */
		QString GetGenericName (const QString& language) const;

		/** @brief Returns the comment of this item.
		 *
		 * @param[in] language The code of the desired language for the
		 * localized comment.
		 * @return The localized comment matching the \em language, or
		 * the default generic name if there is no localized one for the
		 * \em language.
		 */
		QString GetComment (const QString& language) const;

		/** @brief Returns the name of the icon for this item.
		 *
		 * Please note this is not related to the GetIcon() method which
		 * (along with SetIcon()) is provided purely for convenience.
		 *
		 * @return The XDG name of this icon, or a null string if not set.
		 *
		 * @sa GetIcon()
		 */
		QString GetIconName () const;

		/** @brief Returns the categories where this item belongs.
		 *
		 * @return The list of categories for this item.
		 */
		QStringList GetCategories () const;

		/** @brief Returns the type of this item.
		 *
		 * @return The type of this item.
		 */
		Type GetType () const;

		/** @brief Returns type type-specific command for this item.
		 *
		 * A command could be a name of the application for
		 * Type::Application, an URL for Type::URL, and so on.
		 *
		 * @return The type-specific XDG command for this item, or a
		 * null string if not set.
		 *
		 * @sa GetWorkingDirectory()
		 */
		QString GetCommand () const;

		/** @brief Returns the working directory for command execution.
		 *
		 * This directory specifies the working directory where the
		 * command returned by GetCommand() should be executed.
		 *
		 * @return The directory where the command associated with this
		 * item should be executed, or a null string if not set.
		 *
		 * @sa GetCommand()
		 */
		QString GetWorkingDirectory () const;

		/** @brief Returns the permanent ID of this item.
		 *
		 * The returned ID is language-agnostic and is suitable to, for
		 * instance, identify the item in a favorites list.
		 *
		 * @return The permanent ID of this item.
		 */
		QString GetPermanentID () const;

		/** @brief Returns the icon previously set by SetIcon().
		 *
		 * If no icon has been set previously via SetIcon(), this method
		 * returns a null icon even if GetIconName() returns a perfectly
		 * valid name of an existing icon.
		 *
		 * This method is not related to GetIconName() in any way and
		 * (along with SetIcon()) is provided purely for convenience, for
		 * example, to associate a loaded icon with the item.
		 *
		 * @return The icon previously set via SetIcon(), or a null icon
		 * if no icon has been set.
		 *
		 * @sa SetIcon()
		 * @sa GetIconName()
		 */
		QIcon GetIcon (const ICoreProxy_ptr&) const;

		/** @brief Serializes item contents to the debugging \em stream.
		 *
		 * This function is provided for convenience to pretty-print
		 * contents of this item to a debugging stream.
		 *
		 * @param[in] stream The stream to debug-print
		 * @return The debugging \em stream with the contents of this
		 * item.
		 */
		QDebug DebugPrint (QDebug stream) const;

		/** @brief Loads the XDG <code>.desktop</code> item from \em file.
		 *
		 * @param[in] file The file to load the item from.
		 * @return The item loaded from \em file, or an invalid item
		 * (with IsValid() returning <code>false</code>) if the \em file
		 * is invalid.
		 *
		 * @throw std::runtime_error If \em file cannot be opened.
		 */
		static Item_ptr FromDesktopFile (const QString& file);
	};

	/** @brief Serializes \em item contents to the debugging \em stream.
	 *
	 * This function is provided for convenience to pretty-print
	 * contents of \em item to a debugging stream.
	 *
	 * @param[in] stream The debug stream to print to.
	 * @param[in] item The XDG item to print.
	 * @return The debugging \em stream with the contents of the \em item.
	 */
	QDebug operator<< (QDebug stream, const Item& item);
}
