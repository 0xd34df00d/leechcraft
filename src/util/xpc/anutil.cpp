/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "anutil.h"
#include <QObject>
#include <QMap>
#include <interfaces/an/ianemitter.h>
#include <interfaces/an/constants.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>

namespace LC::Util::AN
{
	namespace LAN = LC::AN;

	QMap<QString, QString> GetCategoryNameMap ()
	{
		static const QMap<QString, QString> cat2hr
		{
			{ LAN::CatIM, QObject::tr ("Instant messaging") },
			{ LAN::CatOrganizer, QObject::tr ("Organizer") },
			{ LAN::CatDownloads, QObject::tr ("Downloads") },
			{ LAN::CatPackageManager, QObject::tr ("Package manager") },
			{ LAN::CatMediaPlayer, QObject::tr ("Media player") },
			{ LAN::CatTerminal, QObject::tr ("Terminal") },
			{ LAN::CatNews, QObject::tr ("News") },
			{ LAN::CatGeneric, QObject::tr ("Generic") }
		};
		return cat2hr;
	}

	QStringList GetKnownEventTypes (const QString& category)
	{
		static const QMap<QString, QStringList> cat2types
		{
			{
				LAN::CatIM,
				{
					LAN::TypeIMAttention,
					LAN::TypeIMIncFile,
					LAN::TypeIMIncMsg,
					LAN::TypeIMMUCHighlight,
					LAN::TypeIMMUCInvite,
					LAN::TypeIMMUCMsg,
					LAN::TypeIMStatusChange,
					LAN::TypeIMSubscrGrant,
					LAN::TypeIMSubscrRequest,
					LAN::TypeIMSubscrRevoke,
					LAN::TypeIMSubscrSub,
					LAN::TypeIMSubscrUnsub,
					LAN::TypeIMEventTuneChange,
					LAN::TypeIMEventMoodChange,
					LAN::TypeIMEventActivityChange,
					LAN::TypeIMEventLocationChange
				}
			},
			{
				LAN::CatOrganizer,
				{
					LAN::TypeOrganizerEventDue
				}
			},
			{
				LAN::CatDownloads,
				{
					LAN::TypeDownloadError,
					LAN::TypeDownloadFinished
				}
			},
			{
				LAN::CatPackageManager,
				{
					LAN::TypePackageUpdated
				}
			},
			{
				LAN::CatMediaPlayer,
				{
					LAN::TypeMediaPlaybackStatus
				}
			},
			{
				LAN::CatTerminal,
				{
					LAN::TypeTerminalActivity,
					LAN::TypeTerminalInactivity,
					LAN::TypeTerminalBell
				}
			},
			{
				LAN::CatNews,
				{
					LAN::TypeNewsSourceUpdated,
					LAN::TypeNewsSourceBroken
				}
			},
			{
				LAN::CatGeneric,
				{
					LAN::TypeGeneric
				}
			}
		};
		return cat2types.value (category);
	}

	QString GetCategoryName (const QString& category)
	{
		return GetCategoryNameMap ().value (category, category);
	}

	QString GetTypeName (const QString& type)
	{
		static const QMap<QString, QString> type2hr
		{
			{ LAN::TypeIMAttention, QObject::tr ("Attention request") },
			{ LAN::TypeIMIncFile, QObject::tr ("Incoming file transfer request") },
			{ LAN::TypeIMIncMsg, QObject::tr ("Incoming chat message") },
			{ LAN::TypeIMMUCHighlight, QObject::tr ("MUC highlight") },
			{ LAN::TypeIMMUCInvite, QObject::tr ("MUC invitation") },
			{ LAN::TypeIMMUCMsg, QObject::tr ("General MUC message") },
			{ LAN::TypeIMStatusChange, QObject::tr ("Contact status change") },
			{ LAN::TypeIMSubscrGrant, QObject::tr ("Authorization granted") },
			{ LAN::TypeIMSubscrRevoke, QObject::tr ("Authorization revoked") },
			{ LAN::TypeIMSubscrRequest, QObject::tr ("Authorization requested") },
			{ LAN::TypeIMSubscrSub, QObject::tr ("Contact subscribed") },
			{ LAN::TypeIMSubscrUnsub, QObject::tr ("Contact unsubscribed") },
			{ LAN::TypeIMEventTuneChange, QObject::tr ("Contact's tune changed") },
			{ LAN::TypeIMEventMoodChange, QObject::tr ("Contact's mood changed") },
			{ LAN::TypeIMEventActivityChange, QObject::tr ("Contact's activity changed") },
			{ LAN::TypeIMEventLocationChange, QObject::tr ("Contact's location changed") },

			{ LAN::TypeOrganizerEventDue, QObject::tr ("Event is due") },

			{ LAN::TypeDownloadError, QObject::tr ("Download error") },
			{ LAN::TypeDownloadFinished, QObject::tr ("Download finished") },

			{ LAN::TypePackageUpdated, QObject::tr ("Package updated") },

			{ LAN::TypeMediaPlaybackStatus, QObject::tr ("Media playback status changed") },

			{ LAN::TypeTerminalBell, QObject::tr ("Bell in a terminal") },
			{ LAN::TypeTerminalActivity, QObject::tr ("Activity in a terminal") },
			{ LAN::TypeTerminalInactivity, QObject::tr ("Inactivity in a terminal") },

			{ LAN::TypeNewsSourceUpdated, QObject::tr ("News source got updated") },
			{ LAN::TypeNewsSourceBroken, QObject::tr ("News source is detected to be broken") },

			{ LAN::TypeGeneric, QObject::tr ("Generic") }
		};
		return type2hr.value (type, type);
	}

	QVariant ToVariant (const LC::AN::StringMatcher& matcher)
	{
		const auto value = Util::Visit (matcher,
				[] (const QRegularExpression& expr) { return QVariant { expr }; },
				[] (const auto& wrapper) { return QVariant { wrapper.Pattern_ }; });
		return QVariantMap
		{
			{ "index"_qs, static_cast<int> (matcher.index ()) },
			{ "value"_qs, value },
		};
	}

	LC::AN::StringMatcher StringMatcherFromVariant (const QVariant& var)
	{
		const auto& map = var.toMap ();
		const auto idx = map ["index"_qs].toInt ();
		const auto value = map ["value"_qs];
		switch (idx)
		{
		case 0:
			return LC::AN::Substring { value.toString () };
		case 1:
			return LC::AN::Wildcard { value.toString () };
		case 2:
			return value.toRegularExpression ();
		default:
			qWarning () << "unknown type index" << idx << map;
			return LC::AN::Substring {};
		}
	}

	namespace
	{
		bool GenericMatch (auto&& val, const LC::AN::StringMatcher& pattern)
		{
			const auto pos = Util::Visit (pattern,
					[&val] (const QRegularExpression& rx) { return val.indexOf (rx); },
					[&val] (const LC::AN::Substring& em) { return val.indexOf (em.Pattern_); },
					[&val] (const LC::AN::Wildcard& wc) { return val.indexOf (wc.Compiled_); });
			return pos >= 0;
		}
	}

	bool Matches (const QString& string, const LC::AN::StringMatcher& pattern)
	{
		return GenericMatch (string, pattern);
	}

	bool Matches (const QStringList& strings, const LC::AN::StringMatcher& pattern)
	{
		return GenericMatch (strings, pattern);
	}
}
