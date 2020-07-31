/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QFlags>

namespace LC
{
namespace AN
{
	/** @brief Event cancel pseudo-category.
	 *
	 * This category is used to cancel an event by a given event ID.
	 */
	Q_DECL_IMPORT extern const QString CatEventCancel;

	/** @brief Category of Instant Messaging-related events.
	 */
	Q_DECL_IMPORT extern const QString CatIM;

	/** @brief Another user has requested our user's attention.
	 */
	Q_DECL_IMPORT extern const QString TypeIMAttention;

	/** @brief Another user has sent our user a file.
	 */
	Q_DECL_IMPORT extern const QString TypeIMIncFile;

	/** @brief User has received a message in a standard one-to-one chat.
	 */
	Q_DECL_IMPORT extern const QString TypeIMIncMsg;

	/** @brief User has been highlighted in a multiuser chat.
	 *
	 * The primary difference from TypeIMMUCMsg is that our user must be
	 * explicitly mentioned in another user's message for this event.
	 *
	 * @sa TypeIMMUCMsg
	 */
	Q_DECL_IMPORT extern const QString TypeIMMUCHighlight;

	/** @brief User has been invited to a multiuser chat.
	 */
	Q_DECL_IMPORT extern const QString TypeIMMUCInvite;

	/** @brief A message has been sent to a multiuser chat.
	 *
	 * This event should be emitted for each MUC message, even for those
	 * our user isn't mentioned in.
	 *
	 * @sa TypeIMMUCHighlight
	 */
	Q_DECL_IMPORT extern const QString TypeIMMUCMsg;

	/** @brief Another user in our user's contact list has changed its
	 * status.
	 */
	Q_DECL_IMPORT extern const QString TypeIMStatusChange;

	/** @brief Another user has granted subscription to our user.
	 */
	Q_DECL_IMPORT extern const QString TypeIMSubscrGrant;

	/** @brief Another user has revoked subscription from our user.
	 */
	Q_DECL_IMPORT extern const QString TypeIMSubscrRevoke;

	/** @brief Another user has requested subscription from our user.
	 */
	Q_DECL_IMPORT extern const QString TypeIMSubscrRequest;

	/** @brief Another user has subscribed to our user.
	 */
	Q_DECL_IMPORT extern const QString TypeIMSubscrSub;

	/** @brief Another user has unsubscribed from our user.
	 */
	Q_DECL_IMPORT extern const QString TypeIMSubscrUnsub;

	/** @brief User's tune has changed.
	 */
	Q_DECL_IMPORT extern const QString TypeIMEventTuneChange;

	/** @brief User's mood has changed.
	 */
	Q_DECL_IMPORT extern const QString TypeIMEventMoodChange;

	/** @brief User's activity has changed.
	 */
	Q_DECL_IMPORT extern const QString TypeIMEventActivityChange;

	/** @brief User's location has changed.
	 */
	Q_DECL_IMPORT extern const QString TypeIMEventLocationChange;

	/** @brief Category of Organizer-related events.
	 */
	Q_DECL_IMPORT extern const QString CatOrganizer;

	/** @brief An event due date is coming.
	 */
	Q_DECL_IMPORT extern const QString TypeOrganizerEventDue;

	/** @brief Category of Downloads-related events.
	 */
	Q_DECL_IMPORT extern const QString CatDownloads;

	/** @brief A download has been finished successfully without errors.
	 */
	Q_DECL_IMPORT extern const QString TypeDownloadFinished;

	/** @brief A download has been failed.
	 */
	Q_DECL_IMPORT extern const QString TypeDownloadError;

	/** @brief Category of package manager-related events.
	 */
	Q_DECL_IMPORT extern const QString CatPackageManager;

	/** @brief A package has been updated.
	 */
	Q_DECL_IMPORT extern const QString TypePackageUpdated;

	/** @brief Category of media player-related events.
	 */
	Q_DECL_IMPORT extern const QString CatMediaPlayer;

	/** @brief A media file playback status has been changed.
	 */
	Q_DECL_IMPORT extern const QString TypeMediaPlaybackStatus;

	/** @brief Category for terminal emulation events.
	 */
	Q_DECL_IMPORT extern const QString CatTerminal;

	/** @brief A bell has ringed in a terminal window.
	 */
	Q_DECL_IMPORT extern const QString TypeTerminalBell;

	/** @brief Activity in terminal window.
	 */
	Q_DECL_IMPORT extern const QString TypeTerminalActivity;

	/** @brief Inactivity in terminal window.
	 */
	Q_DECL_IMPORT extern const QString TypeTerminalInactivity;

	/** @brief Category for news-related events.
	 */
	Q_DECL_IMPORT extern const QString CatNews;

	/** @brief News source got updated.
	 */
	Q_DECL_IMPORT extern const QString TypeNewsSourceUpdated;

	/** @brief News source is detected to be broken.
	 */
	Q_DECL_IMPORT extern const QString TypeNewsSourceBroken;

	/** @brief Generic notifications that don't fit into any other category.
	 */
	Q_DECL_IMPORT extern const QString CatGeneric;

	/** @brief Generic type for generic notifications.
	 */
	Q_DECL_IMPORT extern const QString TypeGeneric;

	/** @brief Describes the notification parameters.
	 */
	enum NotifyFlag
	{
		/** @brief No notifications.
		 */
		NotifyNone			= 0,

		/** @brief Rule should be triggered only once.
		 *
		 * This corresponds to the single shot events. That is, after
		 * first triggering of the rule it should be disabled and user
		 * shouldn't get further notifications.
		 */
		NotifySingleShot	= 1 << 0,

		/** @brief User should be notified visually.
		 *
		 * The user should be notified via transient notifications like
		 * a non-intrusive tooltip that will hide soon.
		 *
		 * This is ortogonal to NotifyPersistent.
		 *
		 * @sa NotifyPersistent
		 */
		NotifyTransient		= 1 << 1,

		/** @brief User should be notified visually via persistent
		 * notifications.
		 *
		 * A persistent notification is something like a tray icon
		 * that will be displayed until the user reacts to the event.
		 *
		 * This is ortogonal to NotifyTransient.
		 *
		 * @sa NotifyTransient
		 */
		NotifyPersistent	= 1 << 2,

		/** @brief Notify by playing back an audio file.
		 */
		NotifyAudio			= 1 << 3
	};
	Q_DECLARE_FLAGS (NotifyFlags, NotifyFlag);

	namespace Field
	{
		/** @brief The URL to the file being played.
		*/
		Q_DECL_IMPORT extern const QString MediaPlayerURL;

		/** @brief Playback status of the URL (QString).
		 *
		 * A string, one of:
		 * - Playing
		 * - Paused
		 * - Stopped
		 */
		Q_DECL_IMPORT extern const QString MediaPlaybackStatus;

		/** @brief The title of the currently playing media (QString).
		 */
		Q_DECL_IMPORT extern const QString MediaTitle;

		/** @brief The artist of the currently playing media (QString).
		 */
		Q_DECL_IMPORT extern const QString MediaArtist;

		/** @brief The album of the currently playing media (QString).
		 */
		Q_DECL_IMPORT extern const QString MediaAlbum;

		/** @brief The length of the currently playing media (int).
		 */
		Q_DECL_IMPORT extern const QString MediaLength;

		/** @brief Whether the terminal window is active (bool).
		 */
		Q_DECL_IMPORT extern const QString TerminalActive;

		/** @brief General activity name of a contact (QString).
		 */
		Q_DECL_IMPORT extern const QString IMActivityGeneral;

		/** @brief Specific activity name of a contact (QString).
		 */
		Q_DECL_IMPORT extern const QString IMActivitySpecific;

		/** @brief Accompanying activity text entered by a contact (QString).
		 */
		Q_DECL_IMPORT extern const QString IMActivityText;

		/** @brief General mood name of a contact (QString).
		 */
		Q_DECL_IMPORT extern const QString IMMoodGeneral;

		/** @brief Accompanying mood text entered by a contact (QString).
		 */
		Q_DECL_IMPORT extern const QString IMMoodText;

		/** @brief Longitude of a contact's position (double).
		 */
		Q_DECL_IMPORT extern const QString IMLocationLongitude;

		/** @brief Latitude of a contact's position (double).
		 */
		Q_DECL_IMPORT extern const QString IMLocationLatitude;

		/** @brief Country a contact is currently in (QString).
		 */
		Q_DECL_IMPORT extern const QString IMLocationCountry;

		/** @brief Exact locality, like a town or a city, a contact is
		 * currently in (QString).
		 */
		Q_DECL_IMPORT extern const QString IMLocationLocality;

		/** @brief News source name (QString).
		 */
		Q_DECL_IMPORT extern const QString NewsSourceName;
	}
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::AN::NotifyFlags)
