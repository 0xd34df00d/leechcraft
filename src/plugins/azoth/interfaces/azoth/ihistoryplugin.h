/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <util/threads/coro/taskfwd.h>
#include "imessage.h"

class QDateTime;
class QObject;

namespace LC::Azoth::History
{
	enum class EntryKind : std::uint8_t
	{
		Chat,
		MUC,
		PrivateChat,
	};

	template<template<EntryKind K> typename Kinded>
	using KindedGADT = std::variant<
			Kinded<EntryKind::Chat>,
			Kinded<EntryKind::MUC>,
			Kinded<EntryKind::PrivateChat>
		>;

	template<EntryKind> struct EntryDescr;
	template<> struct EntryDescr<EntryKind::Chat> { QString Nick_; };
	template<> struct EntryDescr<EntryKind::MUC> { QString MucName_; };
	template<> struct EntryDescr<EntryKind::PrivateChat> { QString MucName_; QString Nick_; std::optional<QByteArray> PersistentId_; };

	using SomeEntryDescr = KindedGADT<EntryDescr>;

	template<template<EntryKind K> typename EntryKinded>
	EntryKind GetEntryKind (const KindedGADT<EntryKinded>& gadt)
	{
		return static_cast<EntryKind> (gadt.index ());
	}

	template<EntryKind K>
	struct Entry
	{
		QByteArray AccountId_;
		QString AccountName_;
		QString EntryHumanReadableId_;

		EntryDescr<K> Description_;
	};

	struct SelfEndpoint { QString Name_; std::optional<QString> Variant_; };

	template<EntryKind> struct EntryEndpoint;
	template<> struct EntryEndpoint<EntryKind::Chat> { std::optional<QString> Variant_; };
	template<> struct EntryEndpoint<EntryKind::MUC> { QString Nick_; std::optional<QByteArray> PersistentId_; };
	template<> struct EntryEndpoint<EntryKind::PrivateChat> {};

	template<EntryKind K>
	using Endpoint = std::variant<SelfEndpoint, EntryEndpoint<K>>;

	template<EntryKind K>
	struct EntryDescrWithEndpoint
	{
		EntryDescr<K> Description_;
		Endpoint<K> Endpoint_;
	};
	using SomeEntryDescrWithEndpoint = KindedGADT<EntryDescrWithEndpoint>;

	template<EntryKind K>
	struct NewMessage
	{
		Endpoint<K> Endpoint_;

		QDateTime TS_;
		QString Body_;
		std::optional<QString> RichBody_;
	};

	template<EntryKind K>
	using EntryWithMessages = std::pair<Entry<K>, QList<NewMessage<K>>>;

	using SomeEntryWithMessages = KindedGADT<EntryWithMessages>;
}

namespace LC::Azoth
{
	class IAccount;

	/** @brief Interface for plugins storing chat history.
	 *
	 * This interface should be implemented by plugins that store chat
	 * history for Azoth to provide additional features using these
	 * plugins.
	 */
	class IHistoryPlugin
	{
	protected:
		virtual ~IHistoryPlugin () = default;
	public:
		/** @brief Whether history is enabled for the given entry.
		 *
		 * This method checks if history logging is enabled for the
		 * given entry.
		 *
		 * @param[in] entry The entry to check.
		 * @return Whether history logging is enabled for this entry.
		 */
		virtual bool IsHistoryEnabledFor (ICLEntry& entry) const = 0;

		/** @brief Requests last messages for the given entry.
		 *
		 * This method, when called, requests last num messages from
		 * the chat log with the entry.
		 *
		 * This method is asynchronous: it is expected to return soon
		 * after being called, and the result is expected to be emitted
		 * via the gotLastMessages() signal.
		 *
		 * @param[in] entry The entry for which to query the history.
		 * @param[in] count The maximum number of messages to retrieve.
		 *
		 * @sa gotLastMessages()
		 */
		virtual Util::ContextTask<void> RequestLastMessages (ICLEntry& entry, int count) = 0;

		virtual Util::ContextTask<std::optional<QDateTime>> RequestMaxTimestamp (IAccount& acc) = 0;

		/** @brief Adds a set of messages to the history.
		 */
		virtual void AddMessages (const History::SomeEntryWithMessages&) = 0;
	protected:
		/** @brief Notifies about last messages for the given entry.
		 *
		 * This signal should be emitted when last chat messages with
		 * the given entry have been retrieved from the history as the
		 * result of the call to RequestLastMessages().
		 *
		 * If there are no messages for the entry, the implementation
		 * may either emit this signal with empty messages list or
		 * choose to not emit any signals at all.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @sa RequestLastMessages()
		 */
		virtual void gotLastMessages (QObject *entry, const QList<QObject*>& messages) = 0;
	};
}

Q_DECLARE_INTERFACE (LC::Azoth::IHistoryPlugin,
		"org.Deviant.LeechCraft.Azoth.IHistoryPlugin/1.0")
