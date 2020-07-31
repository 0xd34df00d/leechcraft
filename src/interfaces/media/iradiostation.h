/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>

class QUrl;

namespace Media
{
	struct AudioInfo;

	/** @brief Describes a single radio station.
	 *
	 * Radio stations can be either single-stream or multistream.
	 * Single-stream stations have only one continuous stream of music
	 * like an Icecast/Shoutcast broadcasting. Multistream radio stations
	 * announce tracks one by one, like Last.FM radio. The difference is
	 * that it makes sense to skip a track in multistream radios, while
	 * there is nothing to skip in single-stream ones.
	 *
	 * Multistream radio stations will typically emit gotNewStream()
	 * signals from time to time and as a response to RequestNewStream(),
	 * while single-stream radio stations will emit gotPlaylist() once.
	 *
	 * This class has some signals (gotNewStream(), gotPlaylist() and
	 * gotError()), and one can use the GetQObject() method to get an
	 * object of this class as a QObject and connect to those signals.
	 *
	 * Ownership of this class is either passed to the caller or remains
	 * belonging to the returning plugin, this class never deletes itself.
	 *
	 * @sa IRadioStationProvider
	 * @sa IModifiableRadioStation
	 */
	class Q_DECL_EXPORT IRadioStation
	{
	public:
		virtual ~IRadioStation () {}

		/** @brief Returns this object as a QObject.
		 *
		 * This function can be used to connect to the signals of this
		 * class.
		 *
		 * @return This object as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Request a new stream if this is a multistream radio.
		 *
		 * Otherwise, this function does nothing.
		 */
		virtual void RequestNewStream () = 0;

		/** @brief Returns the human-readable name of this radio station.
		 *
		 * @return The human-readable name of this radio station.
		 */
		virtual QString GetRadioName () const = 0;
	protected:
		/** @brief Emitted when there is a new audio stream available.
		 *
		 * @param[out] url The URL of the stream.
		 * @param[out] info The track metadata.
		 */
		virtual void gotNewStream (const QUrl& url, const AudioInfo& info) = 0;

		/** @brief Emitted when a playlist for this radio is fetched.
		 *
		 * Ownership of the playlist is passed to the owner of this radio
		 * station, and it is responsible for deleting the playlist file
		 * after it is not needed.
		 *
		 * Playlist format is passed in the format parameter, and it can
		 * be one of:
		 * - m3u8
		 * - m3u
		 * - pls
		 * - xspf
		 *
		 * @param[out] filename The local path to the playlist.
		 * @param[out] format The format of the playlist.
		 */
		virtual void gotPlaylist (const QString& filename, const QString& format) = 0;

		/** @brief Emitted when just a set of audio infos is available.
		 *
		 * This signal can be used by radio stations that get audio
		 * streams in a form different of a playlist file.
		 *
		 * The audio infos in the \em infos list must contain the URL
		 * field.
		 *
		 * @param[out] infos The infos corresponding to this radio station.
		 */
		virtual void gotAudioInfos (const QList<AudioInfo>& infos) = 0;

		/** @brief Emitted when there is an error.
		 *
		 * @param[out] error The human-readable error text.
		 */
		virtual void gotError (const QString& error) = 0;
	};

	/** @brief A pointer to a shared instance of a IRadioStation.
	 */
	typedef std::shared_ptr<IRadioStation> IRadioStation_ptr;
}

Q_DECLARE_INTERFACE (Media::IRadioStation, "org.LeechCraft.Media.IRadioStation/1.0")
