/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QtPlugin>
#include "iradiostation.h"

template<typename>
class QFuture;

namespace Media
{
	struct AudioInfo;

	/** @brief Describes the data associated with a radio station restore.
	 *
	 * For now it can be just a list of AudioInfo structs.
	 *
	 * @sa AudioInfo
	 */
	using RadioRestoreVariant_t = std::variant<QList<AudioInfo>>;

	/** @brief Describes the result of restoring a single radio station.
	 *
	 * The radio stations are identified by Media::RadioItemRole::RadioID.
	 */
	struct RadioRestoreResult
	{
		/** @brief The plugin this RadioRestoreResult corresponds to.
		 */
		QString PluginID_;

		/** @brief The radio station this RadioRestoreResult describes.
		 *
		 * This field is equal to some Media::RadioItemRole::RadioID.
		 */
		QString RadioID_;

		/** @brief The restore result itself.
		 */
		RadioRestoreVariant_t Restored_;
	};

	/** @brief A list of RadioRestoreResult structs.
	 */
	using RadiosRestoreResult_t = QList<RadioRestoreResult>;

	/** @brief Interface for radio station providers able to restore the
	 * radio stations between LeechCraft runs.
	 *
	 * An example of such a provider could be the VKontakte streamer. The
	 * links to the songs die after a certain amount of time, but still
	 * it's desirable to allow restoring the list of tracks.
	 *
	 * @sa IRadioStationProvider
	 */
	class IRestorableRadioStationProvider
	{
	protected:
		virtual ~IRestorableRadioStationProvider () {}
	public:
		/** @brief Initiates restoring the radio stations for the given
		 * IDs.
		 *
		 * The IDs are the ones exposed by the provider via the
		 * Media::RadioItemRole::RadioID model role.
		 *
		 * @param[in] ids The list of radio IDs to restore.
		 * @return The future providing a list of RadioRestoreResult
		 * structs.
		 *
		 * @sa RadioRestoreResult
		 */
		virtual QFuture<RadiosRestoreResult_t> RestoreRadioStations (const QStringList& ids) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IRestorableRadioStationProvider, "org.LeechCraft.Media.IRestorableRadioStationProvider/1.0")
