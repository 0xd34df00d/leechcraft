/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "utilconfig.h"
#include <QByteArray>
#include <QSet>
#include <QDataStream>
#include <QtDebug>

namespace LC::Util
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
		explicit IDPool (const T& id = T ())
		: CurrentID_ { id }
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
		 * @param[in] id The ID to free.
		 */
		void FreeID (T id)
		{
			Q_UNUSED (id)
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
