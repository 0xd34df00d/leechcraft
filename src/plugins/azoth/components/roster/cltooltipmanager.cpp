/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cltooltipmanager.h"
#include <QTextDocument>
#include <QUrlQuery>
#include <util/util.h>
#include <util/sll/functional.h>
#include <util/sll/prelude.h>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/xpc/defaulthookproxy.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucperms.h>
#include <interfaces/azoth/iproxyobject.h>
#include "interfaces/azoth/iadvancedclentry.h"
#include "interfaces/azoth/ihaveentitytime.h"
#include "interfaces/azoth/ihavecontacttune.h"
#include "interfaces/azoth/ihavecontactmood.h"
#include "interfaces/azoth/ihavecontactactivity.h"
#include "interfaces/azoth/moodinfo.h"
#include "interfaces/azoth/activityinfo.h"
#include "util/azoth/entryidops.h"
#include "util/azoth/util.h"
#include "components/dialogs/activitydialog.h"
#include "components/dialogs/mooddialog.h"
#include "core.h"
#include "resourcesmanager.h"
#include "avatarsmanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
	namespace
	{
		QUrl GetAvatarUrl (const ICLEntry& entry, const std::optional<QImage>& cached)
		{
			QUrl url;
			url.setScheme ("azoth"_qs);
			url.setHost ("tooltip.avatar"_qs);
			url.setPath ('/' + SerializeString (entry.GetGlobalStrongestId ()), QUrl::DecodedMode);

			if (cached)
			{
				QUrlQuery query;
				query.addQueryItem ("cache", QString::number (cached->cacheKey ()));
				url.setQuery (query);
			}

			return url;
		}

		std::optional<GlobalStrongestId> GetIdFromUrl (const QUrl& url)
		{
			if (url.scheme () != "azoth"_qs || url.host () != "tooltip.avatar"_qs)
				return {};

			return Deserialize<GlobalStrongestId> (url.path (QUrl::FullyDecoded).mid (1));
		}
	}

	CLTooltipManager::CLTooltipManager (AvatarsManager& am)
	: Avatars_ { am }
	{
		auto prevProv = QTextDocument::defaultResourceProvider ();
		QTextDocument::setDefaultResourceProvider ([this, guard = QPointer { this }, prevProv] (const QUrl& url)
		{
			const auto entryId = GetIdFromUrl (url);
			if (!guard || !entryId)
				return prevProv ? prevProv (url) : QVariant {};

			const auto size = XmlSettingsManager::Instance ().property ("CLAvatarsSize").toInt ();
			auto img = Util::Visit (Avatars_.GetAvatarImageNow (*entryId, IAvatarsManager::Scaled { size }),
					[] (const std::optional<QImage>& img) { return img.value_or ({}); },
					[&] (const Util::ContextTask<std::optional<QImage>>& task)
					{
						[] (auto task, CLTooltipManager *mgr, GlobalStrongestId entryId) -> Util::ContextTask<void>
						{
							co_await Util::AddContext { mgr->CoroContext_ };
							if (co_await task)
								emit mgr->redisplayTooltip (entryId);
						} (task, this, *entryId);
						return QImage {};
					});
			if (img.isNull ())
				img = ResourcesManager::Instance ().GetDefaultAvatar (size);
			return QVariant { img };
		});
	}

	namespace
	{
		QString Status2Str (const EntryStatus& status)
		{
			auto result = "<table><tr><td valign='middle'>" + StateToString (status.State_);
			const QString& statusString = status.StatusString_.toHtmlEscaped ();
			if (!statusString.isEmpty ())
				result += " (" + statusString + ")";

			const auto& icon = ResourcesManager::Instance ().GetIconForState (status.State_);
			const auto& data = Util::GetAsBase64Src (icon.pixmap (16, 16).toImage ());
			result += "&nbsp;&nbsp;&nbsp;</td><td><img src='" + data + "' /></td></tr></table>";

			return result;
		}
	}

	void CLTooltipManager::AddEntry (ICLEntry *entry)
	{
		const auto redisplay = [this, entry] { RedisplayTooltip (entry); };

		auto& emitter = entry->GetCLEntryEmitter ();
		connect (&emitter,
				&Emitters::CLEntry::statusChanged,
				this,
				redisplay);
		connect (&emitter,
				&Emitters::CLEntry::availableVariantsChanged,
				this,
				redisplay);
		connect (&emitter,
				&Emitters::CLEntry::entryGenerallyChanged,
				this,
				redisplay);
		connect (&emitter,
				&Emitters::CLEntry::nameChanged,
				this,
				redisplay);
		connect (&emitter,
				&Emitters::CLEntry::groupsChanged,
				this,
				redisplay);

		const auto entryObj = entry->GetQObject ();

		if (const auto perms = qobject_cast<IMUCPerms*> (entryObj))
			connect (&perms->GetMUCPermsEmitter (),
					&Emitters::MUCPerms::permsChanged,
					this,
					[this] (const MucEvents::ParticipantPermsChange& event) { RedisplayTooltip (&event.Participant_); });

		if (qobject_cast<IAdvancedCLEntry*> (entryObj))
		{
			connect (entryObj,
					SIGNAL (attentionDrawn (const QString&, const QString&)),
					this,
					SLOT (redisplayTooltipForSender ()));
			connect (entryObj,
					SIGNAL (locationChanged (const QString&)),
					this,
					SLOT (redisplayTooltipForSender ()));
		}

		if (qobject_cast<IHaveContactTune*> (entryObj))
			connect (entryObj,
					SIGNAL (tuneChanged (QString)),
					this,
					SLOT (redisplayTooltipForSender ()));

		if (qobject_cast<IHaveContactMood*> (entryObj))
			connect (entryObj,
					SIGNAL (moodChanged (QString)),
					this,
					SLOT (redisplayTooltipForSender ()));

		if (qobject_cast<IHaveContactActivity*> (entryObj))
			connect (entryObj,
					SIGNAL (activityChanged (const QString&)),
					this,
					SLOT (redisplayTooltipForSender ()));
	}

	namespace
	{
		void FormatMood (QString& tip, const MoodInfo& moodInfo)
		{
			if (moodInfo.Mood_.isEmpty ())
				return;

			tip += "<br />" + Core::tr ("Mood:") + ' ';
			tip += MoodDialog::ToHumanReadable (moodInfo.Mood_);
			if (!moodInfo.Text_.isEmpty ())
				tip += " (" + moodInfo.Text_ + ")";
		}

		void FormatActivity (QString& tip, const ActivityInfo& actInfo)
		{
			if (actInfo.General_.isEmpty ())
				return;

			tip += "<br />" + Core::tr ("Activity:") + ' ';
			tip += ActivityDialog::ToHumanReadable (actInfo.General_);
			const auto& specific = ActivityDialog::ToHumanReadable (actInfo.Specific_);
			if (!specific.isEmpty ())
				tip += " (" + specific + ")";
			if (!actInfo.Text_.isEmpty ())
				tip += " (" + actInfo.Text_ + ")";
		}

		void FormatTune (QString& tip, const Media::AudioInfo& tuneInfo)
		{
			const auto& artist = tuneInfo.Artist_;
			const auto& source = tuneInfo.Album_;
			const auto& title = tuneInfo.Title_;

			if (artist.isEmpty () && title.isEmpty ())
				return;

			tip += "<br />" + Core::tr ("Now listening to:") + ' ';
			if (!artist.isEmpty () && !title.isEmpty ())
				tip += "<em>" + artist + "</em>" +
						QString::fromUtf8 (" — ") +
						"<em>" + title + "</em>";
			else if (!artist.isEmpty ())
				tip += "<em>" + artist + "</em>";
			else if (!title.isEmpty ())
				tip += "<em>" + title + "</em>";

			if (!source.isEmpty ())
				tip += ' ' + Core::tr ("from") +
						" <em>" + source + "</em>";

			if (tuneInfo.Length_)
				tip += " (" + Util::MakeTimeFromLong (tuneInfo.Length_) + ")";
		}

		void FormatMucPerms (QString& tip, IMUCPerms *mucPerms, ICLEntry& entry)
		{
			if (!mucPerms)
				return;

			tip += "<hr />";
			const auto& perms = mucPerms->GetPerms (entry);
			for (const auto& pair : perms.asKeyValueRange ())
			{
				const auto& permClass = pair.first;

				tip += mucPerms->GetUserString (permClass);
				tip += ": ";

				const auto& users = Util::Map (pair.second,
						Util::BindMemFn (&IMUCPerms::GetUserString, mucPerms));
				tip += users.join ("; ");
				tip += "<br />";
			}
		}
	}

	namespace
	{
		void FormatClientInfo (QString& tip,
				const QMap<QString, QVariant>& info,
				const QIcon& icon)
		{
			QString clientIconString;
			if (!icon.isNull ())
			{
				const auto& data = Util::GetAsBase64Src (icon.pixmap (16, 16).toImage ());
				clientIconString = "&nbsp;&nbsp;&nbsp;<img src='" + data + "'/>";
			}

			bool clientIconInserted = false;

			if (info.contains ("client_name"))
			{
				tip += "<br />" + CLTooltipManager::tr ("Using:") + ' ' + info.value ("client_name").toString ().toHtmlEscaped ();

				if (!info.contains ("client_version"))
				{
					tip += clientIconString;
					clientIconInserted = true;
				}
			}
			if (info.contains ("client_version"))
			{
				tip += " " + info.value ("client_version").toString ().toHtmlEscaped ();

				tip += clientIconString;
				clientIconInserted = true;
			}
			if (info.contains ("client_remote_name"))
			{
				tip += "<br />" + CLTooltipManager::tr ("Claiming:") + ' ' + info.value ("client_remote_name").toString ().toHtmlEscaped ();

				if (!clientIconInserted)
					tip += clientIconString;
			}
			if (info.contains ("client_os"))
				tip += "<br />" + CLTooltipManager::tr ("OS:") + ' ' + info.value ("client_os").toString ().toHtmlEscaped ();
		}
	}

	QString CLTooltipManager::GetTooltip (ICLEntry *entry)
	{
		if (const auto ihet = qobject_cast<IHaveEntityTime*> (entry->GetQObject ()))
			ihet->UpdateEntityTime ();

		QString tip = "<table border='0'><tr><td>";

		const auto& icons = ResourcesManager::Instance ().GetClientIconForEntry (entry);

		if (entry->GetEntryType () != ICLEntry::EntryType::MUC)
		{
			const auto avatarSize = XmlSettingsManager::Instance ().property ("CLAvatarsSize").toInt ();
			const auto cached = Avatars_.GetAvatarImageIfCached (entry->GetGlobalStrongestId (), IAvatarsManager::Scaled { avatarSize });
			tip += "<img src=\"" + GetAvatarUrl (*entry, cached).toString (QUrl::FullyEncoded).toHtmlEscaped () + "\" />";
			tip += "</td><td>";
		}

		tip += "<strong>" + entry->GetEntryName ().toHtmlEscaped () + "</strong>";
		tip += "&nbsp;(<em>" + entry->GetHumanReadableAddress ().toHtmlEscaped () + "</em>)";
		tip += Status2Str (entry->GetStatus ());
		if (entry->GetEntryType () != ICLEntry::EntryType::PrivateChat)
		{
			tip += "<br />";
			tip += tr ("In groups:") + ' ' + entry->Groups ().join ("; ").toHtmlEscaped ();
		}

		const QStringList& variants = entry->Variants ();

		if (auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetParentCLEntryObject ()))
		{
			const QString& jid = mucEntry->GetRealId (*entry);
			tip += "<br />" + tr ("Real ID:") + ' ';
			tip += jid.isEmpty () ? tr ("unknown") : jid.toHtmlEscaped ();
		}

		FormatMucPerms (tip,
				qobject_cast<IMUCPerms*> (entry->GetParentCLEntryObject ()),
				*entry);

		const auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		proxy->SetValue ("tooltip", tip);
		emit hookTooltipBeforeVariants (proxy, entry->GetQObject ());
		proxy->FillValue ("tooltip", tip);

		auto cleanupBR = [&tip] ()
		{
			tip = tip.trimmed ();
			while (tip.endsWith ("<br />"))
			{
				tip.chop (6);
				tip = tip.trimmed ();
			}
		};

		cleanupBR ();

		const auto iHaveTune = qobject_cast<IHaveContactTune*> (entry->GetQObject ());
		const auto iHaveMood = qobject_cast<IHaveContactMood*> (entry->GetQObject ());
		const auto iHaveActivity = qobject_cast<IHaveContactActivity*> (entry->GetQObject ());
		for (const auto& variant : variants)
		{
			const auto& info = entry->GetClientInfo (variant);

			tip += "<hr />";
			if (!variant.isEmpty ())
			{
				tip += "<strong>" + variant;
				if (info.contains ("priority"))
					tip += " (" + QString::number (info.value ("priority").toInt ()) + ")";
				tip += "</strong>";
			}
			if (!variant.isEmpty () || variants.size () > 1)
				tip += Status2Str (entry->GetStatus (variant));

			FormatClientInfo (tip, info, icons.value (variant));

			if (info.contains ("client_time"))
			{
				const auto& datetime = info.value ("client_time").toDateTime ();
				const auto& dateStr = Util::GetLocale ().toString (datetime);
				tip += "<br />" + tr ("Client time:") + ' ' + dateStr;
			}

			if (iHaveActivity)
				FormatActivity (tip, iHaveActivity->GetUserActivity (variant));
			if (iHaveMood)
				FormatMood (tip, iHaveMood->GetUserMood (variant));
			if (iHaveTune)
				FormatTune (tip, iHaveTune->GetUserTune (variant));

			if (info.contains ("custom_user_visible_map"))
			{
				const auto& map = info ["custom_user_visible_map"].toMap ();
				for (const auto& pair : map.asKeyValueRange ())
					tip += "<br />" + pair.first + ": " + pair.second.toString ().toHtmlEscaped () + "<br />";
			}
		}

		cleanupBR ();

		tip += "</td></tr></table>";

		return tip;
	}

	void CLTooltipManager::RedisplayTooltip (ICLEntry *entry)
	{
		emit redisplayTooltip (entry->GetGlobalStrongestId ());
	}

	void CLTooltipManager::redisplayTooltipForSender ()
	{
		RedisplayTooltip (&qobject_ref_cast<ICLEntry> (sender ()));
	}
}
}
