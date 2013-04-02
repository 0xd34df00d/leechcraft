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

#include "utilconfig.h"
#include <QByteArray>
#include <QSet>
#include <QDataStream>
#include <QtDebug>

namespace LeechCraft
{
namespace Util
{
	/** @brief A simple pool of identificators of the given type.
	 *
	 * This class holds a pool of identificators of the given type \em T.
	 * It is very simple and produces consecutive IDs, this \em T should
	 * support <code>operator++()</code>.
	 */
	template<typename T>
	class IDPool
	{
		T CurrentID_;
	public:
		/** @brief Creates a pool with the given initial value.
		 *
		 * @param[in] id The initial value of the pool.
		 */
		IDPool (const T& id = T ())
		: CurrentID_ (id)
		{
		}

		/** @brief Destroys the pool.
		 */
		virtual ~IDPool ()
		{
		}

		/** @brief Returns next ID.
		 *
		 * @return Next ID in the pool.
		 */
		T GetID ()
		{
			return ++CurrentID_;
		}

		/** @brief Forcefully sets the current ID.
		 *
		 * @param[in] id The new current ID.
		 */
		void SetID (T id)
		{
			CurrentID_ = id;
		}

		/** @brief Frees the id.
		 *
		 * If \em id is the last ID issues by the pool, it is reclaimed
		 * and the next id will be \em id again. Otherwise this function does nothing.
		 *
		 * @param[in] id The ID to free.
		 */
		void FreeID (T id)
		{
			if (id == CurrentID_)
				--CurrentID_;
		}

		/** @brief Saves the state of this pool.
		 *
		 * @return The serialized state of this pool.
		 */
		QByteArray SaveState () const
		{
			QByteArray result;
			{
				QDataStream ostr (&result, QIODevice::WriteOnly);
				quint8 ver = 1;
				ostr << ver;
				ostr << CurrentID_;
			}
			return result;
		}

		/** @brief Recovers the state of this pool.
		 *
		 * @param[in] state The state of this pool obtained from
		 * SaveState().
		 */
		void LoadState (const QByteArray& state)
		{
			if (state.isEmpty ())
				return;

			QDataStream istr (state);
			quint8 ver;
			istr >> ver;
			if (ver == 1)
				istr >> CurrentID_;
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown version"
						<< ver
						<< ", not restoring state.";
		}
	};
}
}
