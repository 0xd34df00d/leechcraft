/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <boost/optional.hpp>
#include <QObject>
#include <QDateTime>
#include <QPointer>
#include <QPair>
#include "sllconfig.h"

class QTimer;

namespace LC
{
namespace Util
{
	/** @brief The priority of the action in the queue.
	 */
	enum class QueuePriority
	{
		/** @brief Standard priority.
		 */
		Normal,

		/** @brief Higher priority.
		 */
		High
	};

	/** @brief A simple scheduling manager for a queue of functors.
	 *
	 * This class manages execution of functors that should be called
	 * with some minimal timeout between them.
	 */
	class UTIL_SLL_API QueueManager : public QObject
	{
		Q_OBJECT

		const int Timeout_;
		QTimer * const ReqTimer_;
		QDateTime LastRequest_;

		bool Paused_;

		typedef boost::optional<QPointer<QObject>> OptionalTracker_t;
		QList<QPair<std::function<void ()>, boost::optional<QPointer<QObject>>>> Queue_;
	public:
		/** @brief Creates a queue manager with the given \em timeout.
		 *
		 * @param[in] timeout The timeout between invoking the functions
		 * in milliseconds.
		 * @param[in] parent The parent object of this queue manager.
		 */
		QueueManager (int timeout, QObject *parent = nullptr);

		/** @brief Adds the given \em functor.
		 *
		 * This function adds the given \em functor to the execution
		 * queue, or executes it right at the point of adding if more
		 * than \em timeout has passed since executing the last functor.
		 *
		 * \em dependent is an object this \em functor depends upon. If
		 * \em dependent object is destructed by the time queue reaches
		 * the passed \em functor, the functor will be skipped and next
		 * scheduled functor will be executed (if any).
		 *
		 * @param[in] functor The functor to add to the queue.
		 * @param[in] dependent The dependent object, or nullptr if this
		 * \em functor doesn't depend on anything.
		 * @param[in] prio The priority of the \em functor. Functors with
		 * high priority are added to the beginning of the queue.
		 */
		void Schedule (std::function<void ()> functor,
				QObject *dependent = nullptr,
				QueuePriority prio = QueuePriority::Normal);

		/** @brief Clears the queue.
		 *
		 * Clears the remaining items in the queue, but doesn't abort the
		 * current operation.
		 */
		void Clear ();

		/** @brief Pauses the queue rotation.
		 *
		 * If the queue is already paused, this function does nothing.
		 *
		 * @sa IsPaused(), Resume()
		 */
		void Pause ();

		/** @brief Checks if the queue is paused.
		 *
		 * @return Whether the queue is paused.
		 *
		 * @sa Pause(), Resume()
		 */
		bool IsPaused () const;

		/** @brief Continues the queue rotation.
		 *
		 * If the queue is already running, this function does nothing.
		 *
		 * @sa IsPaused(), Pause()
		 */
		void Resume ();
	private slots:
		void exec ();
	};
}
}
