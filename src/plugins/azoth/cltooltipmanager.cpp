/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cltooltipmanager.h"
#include <QTextDocument>
#include <QStandardItem>
#include <QToolTip>
#include <util/util.h>
#include <util/threads/futures.h>
#include <util/sll/functional.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
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
#include "components/dialogs/activitydialog.h"
#include "components/dialogs/mooddialog.h"
#include "core.h"
#include "resourcesmanager.h"
#include "avatarsmanager.h"
#include "util.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
	CLTooltipManager::CLTooltipManager (AvatarsManager *am, Entry2Items_t& items)
	: AvatarsManager_ { am }
	, Entry2Items_ (items)
	, Avatar2TooltipSrcCache_ { 2 * 1024 * 1024 }
	{
		handleCacheSizeChanged ();
		XmlSettingsManager::Instance ().RegisterObject ("CLToolTipsAvatarsCacheSize",
				this, "handleCacheSizeChanged");

		XmlSettingsManager::Instance ().RegisterObject ("CLAvatarsSize",
				this, "handleAvatarsSizeChanged");
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

	void CLTooltipManager::AddEntry (ICLEntry *clEntry)
	{
		DirtyTooltips_ << clEntry;

		const auto entryObj = clEntry->GetQObject ();

		connect (entryObj,
				SIGNAL (statusChanged (EntryStatus, QString)),
				this,
				SLOT (remakeTooltipForSender ()));
		connect (entryObj,
				SIGNAL (availableVariantsChanged (QStringList)),
				this,
				SLOT (remakeTooltipForSender ()));
		connect (entryObj,
				SIGNAL (entryGenerallyChanged ()),
				this,
				SLOT (remakeTooltipForSender ()));
		connect (entryObj,
				SIGNAL (nameChanged (const QString&)),
				this,
				SLOT (remakeTooltipForSender ()));
		connect (entryObj,
				SIGNAL (groupsChanged (const QStringList&)),
				this,
				SLOT (remakeTooltipForSender ()));
		connect (entryObj,
				SIGNAL (permsChanged ()),
				this,
				SLOT (remakeTooltipForSender ()));

		if (qobject_cast<IHaveAvatars*> (entryObj))
			connect (entryObj,
					SIGNAL (avatarChanged (QObject*)),
					this,
					SLOT (handleAvatarChanged (QObject*)),
					Qt::UniqueConnection);

		if (qobject_cast<IAdvancedCLEntry*> (entryObj))
		{
			connect (entryObj,
					SIGNAL (attentionDrawn (const QString&, const QString&)),
					this,
					SLOT (remakeTooltipForSender ()));
			connect (entryObj,
					SIGNAL (locationChanged (const QString&)),
					this,
					SLOT (remakeTooltipForSender ()));
		}

		if (qobject_cast<IHaveContactTune*> (entryObj))
			connect (entryObj,
					SIGNAL (tuneChanged (QString)),
					this,
					SLOT (remakeTooltipForSender ()));

		if (qobject_cast<IHaveContactMood*> (entryObj))
			connect (entryObj,
					SIGNAL (moodChanged (QString)),
					this,
					SLOT (remakeTooltipForSender ()));

		if (qobject_cast<IHaveContactActivity*> (entryObj))
			connect (entryObj,
					SIGNAL (activityChanged (const QString&)),
					this,
					SLOT (remakeTooltipForSender ()));
	}

	void CLTooltipManager::RemoveEntry (ICLEntry *entry)
	{
		disconnect (entry->GetQObject (),
				0,
				this,
				0);
		DirtyTooltips_.remove (entry);
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

		void FormatMucPerms (QString& tip, IMUCPerms *mucPerms, ICLEntry *entry)
		{
			if (!mucPerms)
				return;

			tip += "<hr />";
			const auto& perms = mucPerms->GetPerms (entry->GetQObject ());
			for (const auto& pair : Util::Stlize (perms))
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

	QString CLTooltipManager::MakeTooltipString (ICLEntry *entry)
	{
		const auto maybeAvatar = Avatar2TooltipSrcCache_ [entry];
		return MakeTooltipString (entry, maybeAvatar ? *maybeAvatar : QString {});
	}

	namespace
	{
		const auto MinAvatarSize = 32;

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

	QString CLTooltipManager::MakeTooltipString (ICLEntry *entry, QString avatarStr)
	{
		QString tip = "<table border='0'><tr><td>";

		const auto& icons = ResourcesManager::Instance ().GetClientIconForEntry (entry);

		bool shouldScheduleAvatarFetch = false;

		const auto avatarSize = XmlSettingsManager::Instance ().property ("CLAvatarsSize").toInt ();

		if (entry->GetEntryType () != ICLEntry::EntryType::MUC)
		{
			if (avatarStr.isNull ())
			{
				avatarStr = Util::GetAsBase64Src (ResourcesManager::Instance ().GetDefaultAvatar (avatarSize));
				shouldScheduleAvatarFetch = true;
				Avatar2TooltipSrcCache_.insert (entry, new QString { avatarStr }, avatarStr.size ());
			}

			tip += "<img src='" + avatarStr + "' />";
			tip += "</td><td>";
		}

		tip += "<strong>" + entry->GetEntryName ().toHtmlEscaped () + "</strong>";
		tip += "&nbsp;(<em>" + entry->GetHumanReadableID ().toHtmlEscaped () + "</em>)";
		tip += Status2Str (entry->GetStatus ());
		if (entry->GetEntryType () != ICLEntry::EntryType::PrivateChat)
		{
			tip += "<br />";
			tip += tr ("In groups:") + ' ' + entry->Groups ().join ("; ").toHtmlEscaped ();
		}

		const QStringList& variants = entry->Variants ();

		if (auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetParentCLEntryObject ()))
		{
			const QString& jid = mucEntry->GetRealID (entry->GetQObject ());
			tip += "<br />" + tr ("Real ID:") + ' ';
			tip += jid.isEmpty () ? tr ("unknown") : jid.toHtmlEscaped ();
		}

		FormatMucPerms (tip,
				qobject_cast<IMUCPerms*> (entry->GetParentCLEntryObject ()),
				entry);

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
				const auto& dateStr = PrettyPrintDateTime (datetime);
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
				for (const auto& pair : Util::Stlize (map))
					tip += "<br />" + pair.first + ": " + pair.second.toString ().toHtmlEscaped () + "<br />";
			}
		}

		cleanupBR ();

		tip += "</td></tr></table>";

		if (shouldScheduleAvatarFetch)
		{
			const auto& obj = entry->GetQObject ();
			Util::Sequence (obj, AvatarsManager_->GetAvatar (obj, IHaveAvatars::Size::Full)) >>
					[this, entry, tip, avatarSize] (QImage avatar)
					{
						if (avatar.isNull ())
							return;

						const auto maxDim = std::max (avatar.width (), avatar.height ());
						if (maxDim > avatarSize)
							avatar = avatar.scaled (avatarSize, avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
						else if (maxDim < MinAvatarSize)
							avatar = avatar.scaled (MinAvatarSize, MinAvatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

						const auto& data = Util::GetAsBase64Src (avatar);
						Avatar2TooltipSrcCache_.insert (entry, new QString { data }, data.size ());

						const auto& newTip = MakeTooltipString (entry, data);
						for (auto item : Entry2Items_.value (entry))
							item->setToolTip (newTip);

						if (QToolTip::isVisible () && QToolTip::text () == tip)
							emit rebuiltTooltip ();
					};
		}

		return tip;
	}

	void CLTooltipManager::RebuildTooltip (ICLEntry *entry)
	{
		if (const auto ihet = qobject_cast<IHaveEntityTime*> (entry->GetQObject ()))
		{
			ihet->UpdateEntityTime ();

			for (const auto& var : entry->Variants ())
				if (entry->GetClientInfo (var).contains ("client_time"))
				{
					DirtyTooltips_ << entry;
					break;
				}
		}

		if (!DirtyTooltips_.contains (entry))
			return;

		const auto& tip = MakeTooltipString (entry);
		for (auto item : Entry2Items_.value (entry))
			item->setToolTip (tip);

		DirtyTooltips_.remove (entry);
	}

	void CLTooltipManager::remakeTooltipForSender ()
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "could not be casted to ICLEntry";
			return;
		}

		for (auto item : Entry2Items_.value (entry))
			item->setToolTip ({});

		DirtyTooltips_ << entry;
	}

	void CLTooltipManager::handleAvatarChanged (QObject *entryObj)
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		DirtyTooltips_ << entry;
		Avatar2TooltipSrcCache_.remove (entry);
	}

	void CLTooltipManager::handleCacheSizeChanged ()
	{
		const auto mibs = XmlSettingsManager::Instance ()
				.property ("CLToolTipsAvatarsCacheSize").toInt ();
		Avatar2TooltipSrcCache_.setMaxCost (mibs * 1024 * 1024);
	}

	void CLTooltipManager::handleAvatarsSizeChanged ()
	{
		Avatar2TooltipSrcCache_.clear ();

		for (const auto& pair : Util::Stlize (Entry2Items_))
			DirtyTooltips_ << pair.first;
	}
}
}
