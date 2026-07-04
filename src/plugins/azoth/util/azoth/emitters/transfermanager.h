/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/itransfermanager.h>
#include "../azothutilconfig.h"

namespace LC::Azoth::Emitters
{
	class AZOTH_UTIL_API TransferJob : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		/** @brief Notifies about transfer progress.
		 *
		 * @param[out] done The amount of data already transferred.
		 * @param[out] total The total amount of data.
		 */
		void transferProgress (qint64 done, qint64 total);

		/** @brief Notifies about state changes.
		 *
		 * @param[out] state The new state of the transfer job.
		 */
		void stateChanged (TransferState state);
	};

	class AZOTH_UTIL_API TransferManager : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		/** @brief Notifies about incoming file transfer offer.
		 *
		 * This signal is emitted by the transfer manager
		 * whenever another party issues a file transfer offer.
		 *
		 * @param[out] offer The file transfer offer.
		 *
		 * @sa offerRevoked
		 */
		void fileOffered (const IncomingOffer& offer);

		/** @brief Notifies the offer is no longer available.
		 *
		 * This signal is emitted by the transfer manager
		 * whenever the offer is no longer available, for instance,
		 * because another party revokes a file transfer offer,
		 * or another party is destroyed.
		 *
		 * @param[out] jobId The ID of the offer being revoked,
		 * matching `IncomingOffer::JobId_`.
		 *
		 * @sa fileOffered
		 */
		void offerRevoked (uint64_t jobId);
	};
}
