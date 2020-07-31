/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QtPlugin>
#include <util/sll/eitherfwd.h>
#include "imessage.h"
#include "ihistoryplugin.h"

class QModelIndex;
class QAbstractItemModel;

template<typename>
class QFuture;

namespace LC
{
namespace Azoth
{
	struct SrvHistMessage
	{
		IMessage::Direction Dir_;
		QByteArray ID_;
		QString Nick_;
		QString Body_;
		QDateTime TS_;

		QString RichBody_;
	};

	using SrvHistMessages_t = QList<SrvHistMessage>;

	enum ServerHistoryRole
	{
		LastMessageDate = Qt::UserRole + 1,
		CLEntry,
		ServerHistoryRoleMax
	};

	enum class ServerHistoryFeature
	{
		AccountSupportsHistory,
		Configurable,
		DatedFetching
	};

	struct DefaultSortParams
	{
		int Column_;
		int Role_;
		Qt::SortOrder Order_;
	};

	class IHaveServerHistory
	{
	public:
		virtual ~IHaveServerHistory () {}

		virtual bool HasFeature (ServerHistoryFeature) const = 0;

		virtual void OpenServerHistoryConfiguration () = 0;

		virtual QAbstractItemModel* GetServerContactsModel () const = 0;

		/** @brief Fetches the given history with the given contact.
		 *
		 * This function should fetch the history with the \em contact,
		 * which is one of the indices of the model returned by
		 * GetServerContactsModel(), and emit the serverHistoryFetched()
		 * signal after fetching.
		 *
		 * A natural ordering is implied on the messages: we say that
		 * one message comes before another one if its date is further
		 * in the past then other's one.
		 *
		 * The history is fetched "around" the message identified by
		 * \em startId. If \em count is positive, then at most
		 * \em count messages should be fetched \em before the message
		 * specified by \em startId. Otherwise if \em count is
		 * negative, <code>abs(count)</code> messages should be fetched
		 * after \em startId.
		 *
		 * If \em startId is empty, then most recent messages should be
		 * fetched.
		 *
		 * @param[in] contact The contact index from the
		 * GetServerContactsModel() to fetch history with.
		 * @param[in] startId The ID of the message around which to fetch
		 * messages, or an empty array if most recent messages are
		 * wanted.
		 * @param[in] count The absolute value of \em count is the number
		 * of messages to fetch. If the value is positive, then messages
		 * before \em startId should be fetched, otherwise messages after
		 * \em startId are to be fetched.
		 *
		 * @sa serverHistoryFetched()
		 */
		virtual void FetchServerHistory (const QModelIndex& contact,
				const QByteArray& startId, int count) = 0;

		virtual DefaultSortParams GetSortParams () const = 0;

		struct UserHistorySyncInfo
		{
			QString VisibleName_;
			QList<HistoryItem> Messages_;
		};
		using MessagesSyncMap_t = QHash<QString, UserHistorySyncInfo>;
		using DatedFetchResult_t = Util::Either<QString, MessagesSyncMap_t>;

		virtual QFuture<DatedFetchResult_t> FetchServerHistory (const QDateTime& since) = 0;
	protected:
		/** @brief Emitted when messages are fetched.
		 *
		 * This signal should be emitted when messages with the given
		 * \em contact are fetched as a result of some previous
		 * FetchServerHistory() call. The \em startId parameter
		 * corresponds to the same-named parameter of
		 * FetchServerHistory(), and \em messages is a list of messages
		 * kept on server (probaly empty).
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] contact The contact with which the server history
		 * is fetched.
		 * @param[out] startId The ID of the message around which the
		 * messages are fetched (see FetchServerHistory() documentation).
		 * @param[out] messages The list of fetched messages (probably
		 * empty). Expected to be sorted in ascending order.
		 */
		virtual void serverHistoryFetched (const QModelIndex& contact,
				const QByteArray& startId, const SrvHistMessages_t& messages) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveServerHistory,
		"org.LeechCraft.Azoth.IHaveServerHistory/1.0")
