/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "../azothutilconfig.h"

namespace LC::Azoth::Emitters
{
	class AZOTH_UTIL_API MUCEntry : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		/** @brief Notifies about subject change.
		 *
		 * This signal should be emitted when room subject is changed
		 * to newSubj.
		 *
		 * @param[out] newSubj The new subject of this room.
		 */
		void mucSubjectChanged (const QString& newSubj);

		/** @brief Notifies about nick conflict.
		 *
		 * This signal should be emitted when room gets the error from
		 * the server that the nickname is already in use.
		 *
		 * The signal handler could either call SetNick() with some
		 * other nickname (in this case the room should automatically
		 * try to rejoin) or do nothing it all (in this case the room
		 * should, well, do nothing as well).
		 *
		 * This signal should be emitted only if the error arises while
		 * joining, not as result of SetNick().
		 *
		 * @param[out] usedNick The nickname that was used to join the
		 * room.
		 */
		void nicknameConflict (const QString& usedNick);

		/** @brief Notifies about us being kicked.
		 *
		 * This signal should be emitted whenever our user gets kicked
		 * from this room.
		 *
		 * @param[out] reason The optional reason message.
		 */
		void beenKicked (const QString& reason);

		/** @brief Notifies about us being banned.
		 *
		 * This signal should be emitted whenever our user gets banned
		 * from this room.
		 *
		 * @param[out] reason The optional reason message.
		 */
		void beenBanned (const QString& reason);
	};
}
