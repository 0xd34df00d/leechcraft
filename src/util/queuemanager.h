/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <functional>
#include <boost/optional.hpp>
#include <QObject>
#include <QDateTime>
#include <QPointer>
#include <QPair>
#include "utilconfig.h"

namespace LeechCraft
{
namespace Util
{
	/** @brief A simple scheduling manager for a queue of functors.
	 *
	 * This class manages execution of functors that should be called
	 * with some minimal timeout between them.
	 */
	class QueueManager : public QObject
	{
		Q_OBJECT

		const int Timeout_;
		QDateTime LastRequest_;

		typedef boost::optional<QPointer<QObject>> OptionalTracker_t;
		QList<QPair<std::function<void ()>, boost::optional<QPointer<QObject>>>> Queue_;
	public:
		/** @brief Creates a queue manager with the given \em timeout.
		 *
		 * @param[in] timeout The timeout between invoking the functions
		 * in milliseconds.
		 * @param[in] parent The parent object of this queue manager.
		 */
		UTIL_API QueueManager (int timeout, QObject *parent = 0);

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
		 */
		UTIL_API void Schedule (std::function<void ()> functor, QObject *dependent = 0);
	private slots:
		void exec ();
	};
}
}
