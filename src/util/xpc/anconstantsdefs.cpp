/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <interfaces/entityconstants.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include <QVariant>

namespace LC
{
	const QString IgnoreSelf { QStringLiteral ("IgnoreSelf") };
	const QString AllowedSemantics { QStringLiteral ("AllowedSemantics") };
}

namespace LC::EF
{
	const QString Priority { QStringLiteral ("Priority") };
	const QString NotificationActions { QStringLiteral ("NotificationActions") };
	const QString NotificationPixmap { QStringLiteral ("NotificationPixmap") };
	const QString NotificationTimeout { QStringLiteral ("NotificationTimeout") };
	const QString HandlingObject { QStringLiteral ("HandlingObject") };
	const QString UserVisibleName { QStringLiteral ("UserVisibleName") };
	const QString Text { QStringLiteral ("Text") };
	const QString Tags { QStringLiteral (" Tags") };

	namespace GlobalAction
	{
		const QString ActionID { QStringLiteral ("ActionID") };
		const QString Shortcut { QStringLiteral ("Shortcut") };
		const QString Receiver { QStringLiteral ("Receiver") };
		const QString Method { QStringLiteral ("Method") };
		const QString AltShortcuts { QStringLiteral ("AltShortcuts") };
	}

	namespace PowerState
	{
		const QString TimeLeft { QStringLiteral ("TimeLeft") };
	}
}

namespace LC::Mimes
{
#define MIME_NOTIFICATION "x-leechcraft/notification"
	const QString Notification { QStringLiteral (MIME_NOTIFICATION) };
	const QString NotificationRuleCreate { QStringLiteral (MIME_NOTIFICATION "-rule-create") };

	const QString GlobalActionRegister { QStringLiteral ("x-leechcraft/global-action-register") };
	const QString GlobalActionUnregister { QStringLiteral ("x-leechcraft/global-action-unregister") };

	const QString PowerStateChanged { QStringLiteral ("x-leechcraft/power-state-changed") };

	const QString DataFilterRequest { QStringLiteral ("x-leechcraft/data-filter-request") };
}

namespace LC::PowerState
{
	const QVariant Sleeping { QByteArray { "Sleeping" } };
	const QVariant WokeUp { QByteArray { "WokeUp" } };
}

namespace LC::AN
{
	const QString CatEventCancel { QStringLiteral ("org.LC.AdvNotifications.Cancel") };

#define CAT_IM "org.LC.AdvNotifications.IM"
	const QString CatIM { QStringLiteral (CAT_IM) };
	const QString TypeIMAttention { QStringLiteral (CAT_IM ".AttentionDrawn") };
	const QString TypeIMIncFile { QStringLiteral (CAT_IM ".IncomingFile") };
	const QString TypeIMIncMsg { QStringLiteral (CAT_IM ".IncomingMessage") };
	const QString TypeIMMUCHighlight { QStringLiteral (CAT_IM ".MUCHighlightMessage") };
	const QString TypeIMMUCInvite { QStringLiteral (CAT_IM ".MUCInvitation") };
	const QString TypeIMMUCMsg { QStringLiteral (CAT_IM ".MUCMessage") };
	const QString TypeIMStatusChange { QStringLiteral (CAT_IM ".StatusChange") };
	const QString TypeIMSubscrGrant { QStringLiteral (CAT_IM ".Subscr.Granted") };
	const QString TypeIMSubscrRevoke { QStringLiteral (CAT_IM ".Subscr.Revoked") };
	const QString TypeIMSubscrRequest { QStringLiteral (CAT_IM ".Subscr.Requested") };
	const QString TypeIMSubscrSub { QStringLiteral (CAT_IM ".Subscr.Subscribed") };
	const QString TypeIMSubscrUnsub { QStringLiteral (CAT_IM ".Subscr.Unsubscribed") };
	const QString TypeIMEventTuneChange { QStringLiteral (CAT_IM ".Event.Tune") };
	const QString TypeIMEventMoodChange { QStringLiteral (CAT_IM ".Event.Mood") };
	const QString TypeIMEventActivityChange { QStringLiteral (CAT_IM ".Event.Activity") };
	const QString TypeIMEventLocationChange { QStringLiteral (CAT_IM ".Event.Location") };

#define CAT_ORGANIZER "org.LC.AdvNotifications.Organizer"
	const QString CatOrganizer { QStringLiteral (CAT_ORGANIZER) };
	const QString TypeOrganizerEventDue { QStringLiteral (CAT_ORGANIZER ".EventDue") };

#define CAT_DOWNLOADS "org.LC.AdvNotifications.Downloads"
	const QString CatDownloads { QStringLiteral (CAT_DOWNLOADS) };
	const QString TypeDownloadFinished { QStringLiteral (CAT_DOWNLOADS ".DownloadFinished") };
	const QString TypeDownloadError { QStringLiteral (CAT_DOWNLOADS ".DownloadError") };

#define CAT_PACKAGE_MANAGER "org.LC.AdvNotifications.PackageManager"
	const QString CatPackageManager { QStringLiteral (CAT_PACKAGE_MANAGER) };
	const QString TypePackageUpdated { QStringLiteral (CAT_PACKAGE_MANAGER ".PackageUpdated") };

#define CAT_MEDIA_PLAYER "org.LC.AdvNotifications.MediaPlayer"
	const QString CatMediaPlayer { QStringLiteral (CAT_MEDIA_PLAYER) };
	const QString TypeMediaPlaybackStatus { QStringLiteral (CAT_MEDIA_PLAYER ".PlaybackStatus") };

#define CAT_TERMINAL "org.LC.AdvNotifications.Terminal"
	const QString CatTerminal { QStringLiteral (CAT_TERMINAL) };
	const QString TypeTerminalBell { QStringLiteral (CAT_TERMINAL ".Bell") };
	const QString TypeTerminalActivity { QStringLiteral (CAT_TERMINAL ".Activity") };
	const QString TypeTerminalInactivity { QStringLiteral (CAT_TERMINAL ".Inactivity") };

#define CAT_NEWS "org.LC.AdvNotifications.News"
	const QString CatNews { QStringLiteral (CAT_NEWS) };
	const QString TypeNewsSourceUpdated { QStringLiteral (CAT_NEWS ".Updated") };
	const QString TypeNewsSourceBroken { QStringLiteral (CAT_NEWS ".Broken") };

#define CAT_GENERIC "org.LC.AdvNotifications.Generic"
	const QString CatGeneric { QStringLiteral (CAT_GENERIC) };
	const QString TypeGeneric { QStringLiteral (CAT_GENERIC ".Generic") };

	namespace Field
	{
		const QString MediaPlayerURL { QStringLiteral (CAT_MEDIA_PLAYER ".Fields.URL") };
		const QString MediaPlaybackStatus { QStringLiteral (CAT_MEDIA_PLAYER ".Fields.PlaybackStatus") };
		const QString MediaTitle { QStringLiteral (CAT_MEDIA_PLAYER ".Fields.Title") };
		const QString MediaArtist { QStringLiteral (CAT_MEDIA_PLAYER ".Fields.Artist") };
		const QString MediaAlbum { QStringLiteral (CAT_MEDIA_PLAYER ".Fields.Album") };
		const QString MediaLength { QStringLiteral (CAT_MEDIA_PLAYER ".Fields.Length") };
		const QString TerminalActive { QStringLiteral (CAT_TERMINAL ".Fields.Active") };
		const QString IMActivityGeneral { QStringLiteral (CAT_IM ".Fields.Activity.General") };
		const QString IMActivitySpecific { QStringLiteral (CAT_IM ".Fields.Activity.Specific") };
		const QString IMActivityText { QStringLiteral (CAT_IM ".Fields.Activity.Text") };
		const QString IMMoodGeneral { QStringLiteral (CAT_IM ".Fields.Mood.General") };
		const QString IMMoodText { QStringLiteral (CAT_IM ".Fields.Mood.Text") };
		const QString IMLocationLongitude { QStringLiteral (CAT_IM ".Fields.Location.Longitude") };
		const QString IMLocationLatitude { QStringLiteral (CAT_IM ".Fields.Location.Latitude") };
		const QString IMLocationCountry { QStringLiteral (CAT_IM ".Fields.Location.Country") };
		const QString IMLocationLocality { QStringLiteral (CAT_IM ".Fields.Location.Locality") };
		const QString NewsSourceName { QStringLiteral (CAT_NEWS ".Fields.Source.Name") };
		const QString NewsSourceURL { QStringLiteral (CAT_NEWS ".Fields.Source.URL") };
	}

	namespace EF
	{
		const QString SenderID { QStringLiteral ("org.LC.AdvNotifications.SenderID") };
		const QString EventCategory { QStringLiteral ("org.LC.AdvNotifications.EventCategory") };
		const QString EventID { QStringLiteral ("org.LC.AdvNotifications.EventID") };
		const QString VisualPath { QStringLiteral ("org.LC.AdvNotifications.VisualPath") };
		const QString EventType { QStringLiteral ("org.LC.AdvNotifications.EventType") };
		const QString FullText { QStringLiteral ("org.LC.AdvNotifications.FullText") };
		const QString ExtendedText { QStringLiteral ("org.LC.AdvNotifications.ExtendedText") };
		const QString DeltaCount { QStringLiteral ("org.LC.AdvNotifications.DeltaCount") };
		const QString Count { QStringLiteral ("org.LC.AdvNotifications.Count") };

		const QString WindowIndex { QStringLiteral ("org.LC.AdvNotifications.WindowIndex") };

		const QString OpenConfiguration { QStringLiteral ("org.LC.AdvNotifications.OpenConfiguration") };
		const QString IsSingleShot { QStringLiteral ("org.LC.AdvNotifications.SingleShot") };
		const QString NotifyTransient { QStringLiteral ("org.LC.AdvNotifications.NotifyTransient") };
		const QString NotifyPersistent { QStringLiteral ("org.LC.AdvNotifications.NotifyPersistent") };
		const QString NotifyAudio { QStringLiteral ("org.LC.AdvNotifications.NotifyAudio") };

		const QString RuleID { QStringLiteral ("org.LC.AdvNotifications.RuleID") };
		const QString AssocColor { QStringLiteral ("org.LC.AdvNotifications.AssocColor") };
		const QString IsEnabled { QStringLiteral ("org.LC.AdvNotifications.IsEnabled") };
	}
}
