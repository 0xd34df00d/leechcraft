/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include "xpcconfig.h"

namespace LC::AN
{
	const QString CatEventCancel { "org.LC.AdvNotifications.Cancel" };

	const QString CatIM { "org.LC.AdvNotifications.IM" };
	const QString TypeIMAttention { CatIM + ".AttentionDrawn" };
	const QString TypeIMIncFile { CatIM + ".IncomingFile" };
	const QString TypeIMIncMsg { CatIM + ".IncomingMessage" };
	const QString TypeIMMUCHighlight { CatIM + ".MUCHighlightMessage" };
	const QString TypeIMMUCInvite { CatIM + ".MUCInvitation" };
	const QString TypeIMMUCMsg { CatIM + ".MUCMessage" };
	const QString TypeIMStatusChange { CatIM + ".StatusChange" };
	const QString TypeIMSubscrGrant { CatIM + ".Subscr.Granted" };
	const QString TypeIMSubscrRevoke { CatIM + ".Subscr.Revoked" };
	const QString TypeIMSubscrRequest { CatIM + ".Subscr.Requested" };
	const QString TypeIMSubscrSub { CatIM + ".Subscr.Subscribed" };
	const QString TypeIMSubscrUnsub { CatIM + ".Subscr.Unsubscribed" };
	const QString TypeIMEventTuneChange { CatIM + ".Event.Tune" };
	const QString TypeIMEventMoodChange { CatIM + ".Event.Mood" };
	const QString TypeIMEventActivityChange { CatIM + ".Event.Activity" };
	const QString TypeIMEventLocationChange { CatIM + ".Event.Location" };

	const QString CatOrganizer { "org.LC.AdvNotifications.Organizer" };
	const QString TypeOrganizerEventDue { CatOrganizer + ".EventDue" };

	const QString CatDownloads { "org.LC.AdvNotifications.Downloads" };
	const QString TypeDownloadFinished { CatDownloads + ".DownloadFinished" };
	const QString TypeDownloadError { CatDownloads + ".DownloadError" };

	const QString CatPackageManager { "org.LC.AdvNotifications.PackageManager" };
	const QString TypePackageUpdated { CatPackageManager + ".PackageUpdated" };

	const QString CatMediaPlayer { "org.LC.AdvNotifications.MediaPlayer" };
	const QString TypeMediaPlaybackStatus { CatMediaPlayer + ".PlaybackStatus" };

	const QString CatTerminal { "org.LC.AdvNotifications.Terminal" };
	const QString TypeTerminalBell { CatTerminal + ".Bell" };
	const QString TypeTerminalActivity { CatTerminal + ".Activity" };
	const QString TypeTerminalInactivity { CatTerminal + ".Inactivity" };

	const QString CatNews { "org.LC.AdvNotifications.News" };
	const QString TypeNewsSourceUpdated { CatNews + ".Updated" };
	const QString TypeNewsSourceBroken { CatNews + ".Broken" };

	const QString CatGeneric { "org.LC.AdvNotifications.Generic" };
	const QString TypeGeneric { CatGeneric + ".Generic" };

	namespace Field
	{
		const QString MediaPlayerURL { CatMediaPlayer + ".Fields.URL" };
		const QString MediaPlaybackStatus { CatMediaPlayer + ".Fields.PlaybackStatus" };
		const QString MediaTitle { CatMediaPlayer + ".Fields.Title" };
		const QString MediaArtist { CatMediaPlayer + ".Fields.Artist" };
		const QString MediaAlbum { CatMediaPlayer + ".Fields.Album" };
		const QString MediaLength { CatMediaPlayer + ".Fields.Length" };
		const QString TerminalActive { CatTerminal + ".Fields.Active" };
		const QString IMActivityGeneral { CatIM + ".Fields.Activity.General" };
		const QString IMActivitySpecific { CatIM + ".Fields.Activity.Specific" };
		const QString IMActivityText { CatIM + ".Fields.Activity.Text" };
		const QString IMMoodGeneral { CatIM + ".Fields.Mood.General" };
		const QString IMMoodText { CatIM + ".Fields.Mood.Text" };
		const QString IMLocationLongitude { CatIM + ".Fields.Location.Longitude" };
		const QString IMLocationLatitude { CatIM + ".Fields.Location.Latitude" };
		const QString IMLocationCountry { CatIM + ".Fields.Location.Country" };
		const QString IMLocationLocality { CatIM + ".Fields.Location.Locality" };
		const QString NewsSourceName { CatNews + ".Fields.Source.Name" };
	}

	namespace EF
	{
		const QString SenderID { "org.LC.AdvNotifications.SenderID" };
		const QString EventCategory { "org.LC.AdvNotifications.EventCategory" };
		const QString EventID { "org.LC.AdvNotifications.EventID" };
		const QString VisualPath { "org.LC.AdvNotifications.VisualPath" };
		const QString EventType { "org.LC.AdvNotifications.EventType" };
		const QString FullText { "org.LC.AdvNotifications.FullText" };
		const QString ExtendedText { "org.LC.AdvNotifications.ExtendedText" };
		const QString DeltaCount { "org.LC.AdvNotifications.DeltaCount" };
		const QString Count { "org.LC.AdvNotifications.Count" };
		const QString OpenConfiguration { "org.LC.AdvNotifications.OpenConfiguration" };
		const QString IsSingleShot { "org.LC.AdvNotifications.SingleShot" };
		const QString NotifyTransient { "org.LC.AdvNotifications.NotifyTransient" };
		const QString NotifyPersistent { "org.LC.AdvNotifications.NotifyPersistent" };
		const QString NotifyAudio { "org.LC.AdvNotifications.NotifyAudio" };
	}
}
