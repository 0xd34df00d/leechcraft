/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "anutil.h"
#include <algorithm>
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

	using SVM = LC::AN::StringValueMatcher;

	QVariant ToVariant (const SVM::Pattern& matcher)
	{
		const auto value = Visit (matcher,
				[] (const QRegularExpression& expr) { return QVariant { expr }; },
				[] (const auto& wrapper) { return QVariant { wrapper.Pattern_ }; });
		return QVariantMap
		{
			{ "index"_qs, static_cast<int> (matcher.index ()) },
			{ "value"_qs, value },
		};
	}

	std::optional<SVM::Pattern> StringPatternFromVariant (const QVariant& var)
	{
		const auto& map = var.toMap ();
		const auto idx = map ["index"_qs].toInt ();
		const auto value = map ["value"_qs];
		switch (idx)
		{
		case 0:
			return SVM::Substring { value.toString () };
		case 1:
			return SVM::Wildcard { value.toString () };
		case 2:
			return value.toRegularExpression ();
		case 3:
			return SVM::Exact { value.toString () };
		default:
			return {};
		}
	}

	namespace
	{
		bool MatchStringPattern (const QString& string, const SVM::Pattern& matcher)
		{
			return Visit (matcher,
					[&] (const QRegularExpression& rx) { return string.contains (rx); },
					[&] (const SVM::Substring& em) { return string.contains (em.Pattern_); },
					[&] (const SVM::Wildcard& wc) { return string.contains (wc.Compiled_); },
					[&] (const SVM::Exact& em) { return string == em.Pattern_; });
		}
	}

	bool Matches (const QString& string, const SVM::Pattern& matcher)
	{
		return MatchStringPattern (string, matcher);
	}

	bool Matches (const QStringList& strings, const SVM::Pattern& matcher)
	{
		return std::ranges::any_of (strings, [&matcher] (const QString& str) { return MatchStringPattern (str, matcher); });
	}

	bool Matches (const QString& string, const LC::AN::StringValueMatcher& matcher)
	{
		return MatchStringPattern (string, matcher.Pattern_) == matcher.Positive_;
	}

	bool Matches (const QStringList& strings, const LC::AN::StringValueMatcher& matcher)
	{
		return std::ranges::any_of (strings, [&matcher] (const QString& str) { return MatchStringPattern (str, matcher.Pattern_); }) == matcher.Positive_;
	}

	bool Matches (const QUrl& url, const LC::AN::StringValueMatcher& matcher)
	{
		const auto strMatches = MatchStringPattern (url.toString (), matcher.Pattern_)
				|| MatchStringPattern (QString::fromLatin1 (url.toEncoded ()), matcher.Pattern_);
		return strMatches == matcher.Positive_;
	}

	bool Matches (bool value, const LC::AN::BoolValueMatcher& matcher)
	{
		return value == matcher.Value_;
	}

	bool Matches (int value, const LC::AN::IntValueMatcher& matcher)
	{
		using enum LC::AN::IntValueMatcher::Operation;

		if ((matcher.Ops_ & OEqual) && value == matcher.Boundary_)
			return true;
		if ((matcher.Ops_ & OGreater) && value > matcher.Boundary_)
			return true;
		if ((matcher.Ops_ & OLess) && value < matcher.Boundary_)
			return true;

		return false;
	}

	bool Matches (const QVariant& var, QMetaType::Type expected, const LC::AN::ValueMatcher& matcher)
	{
		if (var.metaType ().id () != expected)
		{
			qCritical () << "expected" << QMetaType { expected } << "; got" << var.metaType () << var;
			return false;
		}

		const auto ensureExpected = [&] (QMetaType::Type matcherType)
		{
			if (expected == matcherType)
				return true;
			qCritical () << "expected metatype" << QMetaType { expected } << "for matcher" << QMetaType { matcherType };
			return false;
		};

		using namespace LC::AN;
		return Visit (matcher,
				[&] (const BoolValueMatcher& m) { return ensureExpected (QMetaType::Bool) && Matches (var.toBool (), m); },
				[&] (const IntValueMatcher& m) { return ensureExpected (QMetaType::Int) && Matches (var.toInt (), m); },
				[&] (const StringValueMatcher& m)
				{
					switch (expected)
					{
					case QMetaType::QString:
						return Matches (var.toString (), m);
					case QMetaType::QStringList:
						return Matches (var.toStringList (), m);
					case QMetaType::QUrl:
						return Matches (var.toUrl (), m);
					default:
						qCritical () << "unexpected metatype" << QMetaType { expected } << "for string matcher";
						return false;
					}
				});
	}
}
