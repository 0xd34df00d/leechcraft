/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationsmanager.h"
#include <QMainWindow>
#include <util/gui/util.h>
#include <util/sll/prelude.h>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xpc/defaulthookproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/media/audiostructs.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/imessage.h"
#include "interfaces/azoth/iadvancedclentry.h"
#include "interfaces/azoth/iextselfinfoaccount.h"
#include "interfaces/azoth/isupportmood.h"
#include "interfaces/azoth/isupportactivity.h"
#include "interfaces/azoth/isupportgeolocation.h"
#include "interfaces/azoth/ihavecontacttune.h"
#include "interfaces/azoth/ihavecontactmood.h"
#include "interfaces/azoth/ihavecontactactivity.h"
#include "interfaces/azoth/moodinfo.h"
#include "interfaces/azoth/activityinfo.h"
#include "util/azoth/util.h"
#include "components/dialogs/activitydialog.h"
#include "components/dialogs/joinconferencedialog.h"
#include "components/dialogs/mooddialog.h"
#include "components/util/entries.h"
#include "components/util/misc.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "chattabsmanager.h"
#include "proxyobject.h"
#include "avatarsmanager.h"

namespace LC
{
namespace Azoth
{
	namespace Fields
	{
		const auto SubSourceID = "org.LC.Plugins.Azoth.SubSourceID"_qs;
		const auto Msg = "org.LC.Plugins.Azoth.Msg"_qs;
		const auto NewStatus = "org.LC.Plugins.Azoth.NewStatus"_qs;
		const auto SourceName = "org.LC.Plugins.Azoth.SourceName"_qs;
		const auto SourceID = "org.LC.Plugins.Azoth.SourceID"_qs;
		const auto ParentSourceName = "org.LC.Plugins.Azoth.ParentSourceName"_qs;
		const auto ParentSourceID = "org.LC.Plugins.Azoth.ParentSourceID"_qs;
		const auto SourceGroups = "org.LC.Plugins.Azoth.SourceGroups"_qs;
	}

	QList<AN::FieldData> NotificationsManager::GetANFields ()
	{
		const QStringList havingMsgField
		{
			AN::TypeIMMUCHighlight,
			AN::TypeIMMUCMsg,
			AN::TypeIMIncMsg,
			AN::TypeIMIncFile,
			AN::TypeIMAttention,
			AN::TypeIMSubscrGrant,
			AN::TypeIMSubscrRevoke,
			AN::TypeIMSubscrRequest
		};

		const QStringList havingSourceFields
		{
			AN::TypeIMMUCHighlight,
			AN::TypeIMMUCMsg,
			AN::TypeIMIncMsg,
			AN::TypeIMIncFile,
			AN::TypeIMAttention,
			AN::TypeIMSubscrGrant,
			AN::TypeIMSubscrRevoke,
			AN::TypeIMSubscrRequest,
			AN::TypeIMStatusChange,
			AN::TypeIMEventTuneChange,
			AN::TypeIMEventMoodChange,
			AN::TypeIMEventActivityChange,
			AN::TypeIMEventLocationChange
		};

		return
		{
			{
				.ID_ = Fields::Msg,
				.Name_ = tr ("Message body"),
				.Description_ = tr ("Original human-readable message body."),
				.Type_ = QMetaType::QString,
				.EventTypes_ = [&]
				{
					auto res = havingMsgField + havingSourceFields;
					res.removeDuplicates ();
					return res;
				} ()
			},
			{
				.ID_ = Fields::SourceName,
				.Name_ = tr ("Sender name"),
				.Description_ = tr ("Human-readable name of the sender of the message."),
				.Type_ = QMetaType::QString,
				.EventTypes_ = havingSourceFields,
			},
			{
				.ID_ = Fields::SourceID,
				.Name_ = tr ("Sender ID"),
				.Description_ = tr ("Non-human-readable ID of the sender (protocol-specific)."),
				.Type_ = QMetaType::QString,
				.EventTypes_ = havingSourceFields,
			},
			{
				.ID_ = Fields::ParentSourceName,
				.Name_ = tr ("Sender's parent entry name"),
				.Description_ = tr ("Human-readable name of the parent entry of the sender of the message, like MUC name for a chat participant."),
				.Type_ = QMetaType::QString,
				.EventTypes_ = havingSourceFields,
			},
			{
				.ID_ = Fields::ParentSourceID,
				.Name_ = tr ("Sender's parent ID"),
				.Description_ = tr ("Non-human-readable ID of the parent entry of the sender of the message, like MUC name for a chat participant."),
				.Type_ = QMetaType::QString,
				.EventTypes_ = havingSourceFields,
			},
			{
				.ID_ = Fields::SourceGroups,
				.Name_ = tr ("Sender groups"),
				.Description_ = tr ("Groups to which the sender belongs."),
				.Type_ = QMetaType::QStringList,
				.EventTypes_ = havingSourceFields,
			},
			{
				.ID_ = Fields::NewStatus,
				.Name_ = tr ("New status"),
				.Description_ = tr ("The new status string of the contact."),
				.Type_ = QMetaType::QString,
				.EventTypes_ = { AN::TypeIMStatusChange },
				.AllowedValues_ = Util::Map (QList { SOnline, SChat, SAway, SDND, SXA, SOffline },
						[] (State st) { return AN::FieldData::AllowedValue { StateToID (st).toUtf8 (), StateToString (st) }; }),
			}
		};
	}

	NotificationsManager::NotificationsManager (IEntityManager *manager, AvatarsManager *am, QObject *parent)
	: QObject { parent }
	, EntityMgr_ { manager }
	, AvatarsMgr_ { am }
	{
	}

	namespace
	{
		const auto EventTitle = "Azoth"_qs;

		auto NotifyWithReason (AvatarsManager& avatarsMgr,
				const char *settingName,
				const QString& eventType,
				const QString& patternLite,
				const QString& patternFull)
		{
			return [=, &avatarsMgr] (ICLEntry& entry, const QString& msg)
			{
				if (!XmlSettingsManager::Instance ().property (settingName).toBool ())
					return;

				const auto& name = Util::FormatName (entry.GetEntryName ());
				const auto& address = entry.GetHumanReadableAddress ();
				const auto& str = msg.isEmpty () ?
						patternLite.arg (name, address) :
						patternFull.arg (name, address, msg);

				auto e = Util::MakeNotification (EventTitle, str, Priority::Info);
				e.Additional_ [AN::EF::EventType] = eventType;
				e.Additional_ [AN::EF::FullText] = str;
				e.Additional_ [AN::EF::Count] = 1;
				e.Additional_ [Fields::Msg] = msg;

				Util::Sequence (entry.GetQObject (), BuildNotification (&avatarsMgr, e, &entry, "Event")) >>
						[] (const Entity& e) { GetProxyHolder ()->GetEntityManager ()->HandleEntity (e); };
			};
		}

		void NotifyNonRosterUnsub (const QString& entryId, const QString& msg)
		{
			if (!XmlSettingsManager::Instance ().property ("NotifyAboutNonrosterUnsub").toBool ())
				return;

			const auto& name = Util::FormatName (entryId);
			const auto& str = msg.isEmpty () ?
					NotificationsManager::tr ("%1 unsubscribed from us.").arg (name) :
					NotificationsManager::tr ("%1 unsubscribed from us: %2.").arg (name, msg);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification (EventTitle, str, Priority::Info));
		}

		void NotifyAuthRequested (AvatarsManager& avatarsMgr, ICLEntry& entry, const QString& msg)
		{
			/* TODO go through new typed hooks
			const auto& proxy = std::make_shared<Util::DefaultHookProxy> ();
			emit hookGotAuthRequest (proxy, entryObj, msg);
			if (proxy->IsCancelled ())
				return;
				*/

			const auto& name = Util::FormatName (entry.GetEntryName ());
			const auto& str = msg.isEmpty () ?
					NotificationsManager::tr ("%1 requested subscription.").arg (name) :
					NotificationsManager::tr ("%1 requested subscription: %2.").arg (name, msg);
			auto e = Util::MakeNotification (EventTitle, str, Priority::Info);
			e.Additional_ [AN::EF::EventType] = AN::TypeIMSubscrRequest;
			e.Additional_ [AN::EF::FullText] = str;
			e.Additional_ [AN::EF::Count] = 1;
			e.Additional_ [Fields::Msg] = msg;

			const auto nh = new Util::NotificationActionHandler { e };
			nh->AddFunction (NotificationsManager::tr ("Authorize"), [&entry] { AuthorizeEntry (&entry); });
			nh->AddFunction (NotificationsManager::tr ("Deny"), [&entry] { DenyAuthForEntry (&entry); });
			nh->AddFunction (NotificationsManager::tr ("View info"), [&entry] { entry.ShowInfo (); });
			nh->AddDependentObject (entry.GetQObject ());

			Util::Sequence (entry.GetQObject (), BuildNotification (&avatarsMgr, e, &entry, "AuthRequestFrom")) >>
					[] (const Entity& e) { GetProxyHolder ()->GetEntityManager ()->HandleEntity (e); };
		}

		void SuggestJoiningMUC (IAccount *acc, QVariantMap ident)
		{
			if (auto& accountId = ident ["AccountID"];
				accountId.toByteArray ().isEmpty ())
				accountId = acc->GetAccountID ();

			const auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
			const auto dia = new JoinConferenceDialog { { acc }, rootWM->GetPreferredWindow () };
			dia->SetIdentifyingData (ident);
			dia->show ();
		}

		void HandleMUCInvitation (IAccount& acc, const QVariantMap& ident, const QString& inviter, const QString& reason)
		{
			const auto& name = ident ["HumanReadableName"].toString ();

			const auto str = reason.isEmpty () ?
					NotificationsManager::tr ("You have been invited to %1 by %2.")
							.arg (Util::FormatName (name), Util::FormatName (inviter)) :
					NotificationsManager::tr ("You have been invited to %1 by %2: %3")
							.arg (Util::FormatName (name), Util::FormatName (inviter), reason);

			auto e = Util::MakeNotification (EventTitle, str, Priority::Info);
			e.Additional_ [AN::EF::SenderID] = "org.LeechCraft.Azoth";
			e.Additional_ [AN::EF::EventCategory] = AN::CatIM;
			e.Additional_ [AN::EF::VisualPath] = QStringList (name);
			e.Additional_ [AN::EF::EventID] = "org.LC.Plugins.Azoth.Invited/" + name + '/' + inviter;
			e.Additional_ [AN::EF::EventType] = AN::TypeIMMUCInvite;
			e.Additional_ [AN::EF::FullText] = str;
			e.Additional_ [AN::EF::Count] = 1;
			e.Additional_ [Fields::Msg] = reason;

			const auto& cancel = Util::MakeANCancel (e);

			const auto nh = new Util::NotificationActionHandler { e };
			nh->AddFunction (NotificationsManager::tr ("Join"),
					[&acc, ident, cancel]
					{
						SuggestJoiningMUC (&acc, ident);
						GetProxyHolder ()->GetEntityManager ()->HandleEntity (cancel);
					});
			nh->AddDependentObject (acc.GetQObject ());

			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}
	}

	void NotificationsManager::AddAccount (QObject *accObject)
	{
		auto& acc = qobject_ref_cast<IAccount> (accObject);
		auto& emitter = acc.GetAccountEmitter ();
		connect (&emitter,
				&Emitters::Account::itemSubscribed,
				this,
				NotifyWithReason (*AvatarsMgr_, "NotifySubscriptions", AN::TypeIMSubscrSub,
						tr ("%1 (%2) subscribed to us."),
						tr ("%1 (%2) subscribed to us: %3.")));
		connect (&emitter,
				qOverload<ICLEntry&, const QString&> (&Emitters::Account::itemUnsubscribed),
				this,
				NotifyWithReason (*AvatarsMgr_, "NotifyUnsubscriptions", AN::TypeIMSubscrUnsub,
						tr ("%1 (%2) unsubscribed from us."),
						tr ("%1 (%2) unsubscribed from us: %3.")));
		connect (&emitter,
				qOverload<const QString&, const QString&> (&Emitters::Account::itemUnsubscribed),
				this,
				&NotifyNonRosterUnsub);
		connect (&emitter,
				&Emitters::Account::itemCancelledSubscription,
				this,
				NotifyWithReason (*AvatarsMgr_, "NotifySubCancels", AN::TypeIMSubscrRevoke,
						tr ("%1 (%2) cancelled our subscription."),
						tr ("%1 (%2) cancelled our subscription: %3.")));
		connect (&emitter,
				&Emitters::Account::itemGrantedSubscription,
				this,
				NotifyWithReason (*AvatarsMgr_, "NotifySubGrants", AN::TypeIMSubscrGrant,
						tr ("%1 (%2) granted subscription."),
						tr ("%1 (%2) granted subscription: %3.")));
		connect (&emitter,
				&Emitters::Account::authorizationRequested,
				this,
				std::bind_front (&NotifyAuthRequested, std::ref (*AvatarsMgr_)));

		connect (&emitter,
				&Emitters::Account::mucInvitationReceived,
				this,
				std::bind_front (&HandleMUCInvitation, std::ref (acc)));

		connect (&emitter,
				&Emitters::Account::statusChanged,
				this,
				[this, &acc] (const EntryStatus& status)
				{
					if (status.State_ == SOffline)
						LastAccountStatusChange_.remove (&acc);
					else if (!LastAccountStatusChange_.contains (&acc))
						LastAccountStatusChange_ [&acc] = QDateTime::currentDateTime ();
				});
	}

	void NotificationsManager::RemoveAccount (QObject *accObject)
	{
		qobject_ref_cast<IAccount> (accObject).GetAccountEmitter ().disconnect (this);
	}

	void NotificationsManager::AddCLEntry (QObject *entryObj)
	{
		auto& entry = qobject_ref_cast<ICLEntry> (entryObj);
		auto& emitter = entry.GetCLEntryEmitter ();
		connect (&emitter,
				&Emitters::CLEntry::statusChanged,
				this,
				std::bind_front (&NotificationsManager::HandleStatusChanged, this, &entry));
		connect (&emitter,
				&Emitters::CLEntry::chatPartStateChanged,
				this,
				std::bind_front (&NotificationsManager::HandleChatPartStateChanged, this, &entry));

		if (qobject_cast<IAdvancedCLEntry*> (entryObj))
		{
			connect (entryObj,
					SIGNAL (attentionDrawn (const QString&, const QString&)),
					this,
					SLOT (handleAttentionDrawn (const QString&, const QString&)));
			connect (entryObj,
					SIGNAL (locationChanged (QString)),
					this,
					SLOT (handleLocationChanged (QString)));
		}

		if (qobject_cast<IHaveContactTune*> (entryObj))
			connect (entryObj,
					SIGNAL (tuneChanged (QString)),
					this,
					SLOT (handleTuneChanged (QString)));

		if (qobject_cast<IHaveContactMood*> (entryObj))
			connect (entryObj,
					SIGNAL (moodChanged (QString)),
					this,
					SLOT (handleMoodChanged (QString)));

		if (qobject_cast<IHaveContactActivity*> (entryObj))
			connect (entryObj,
					SIGNAL (activityChanged (QString)),
					this,
					SLOT (handleActivityChanged (QString)));

		const auto status = entry.GetStatus ();
		if (status.State_ != SOffline)
			HandleStatusChanged (&entry, status, entry.Variants ().value (0));
	}

	void NotificationsManager::RemoveCLEntry (QObject *entryObj)
	{
		disconnect (entryObj,
				0,
				this,
				0);

		if (const auto entry = qobject_cast<ICLEntry*> (entryObj))
			entry->GetCLEntryEmitter ().disconnect (this);
	}

	void NotificationsManager::HandleMessage (IMessage *msg)
	{
		const bool showMsg = XmlSettingsManager::Instance ()
				.property ("ShowMsgInNotifications").toBool ();

		const auto other = qobject_cast<ICLEntry*> (msg->OtherPart ());
		const auto parentCL = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());

		const auto& body = FormatterProxyObject {}.EscapeBody (msg->GetBody (), msg->GetEscapePolicy ());

		auto title = EventTitle;
		const auto fmtEntryName = Util::FormatName (other->GetEntryName ());

		QString msgString;
		bool isHighlightMsg = false;

		switch (msg->GetMessageType ())
		{
		case IMessage::Type::ChatMessage:
			if (XmlSettingsManager::Instance ().property ("NotifyAboutIncomingMessages").toBool ())
			{
				if (showMsg)
				{
					title = other->GetEntryName ();
					msgString = body;
				}
				else
					msgString = tr ("%1 sent you a message.").arg (fmtEntryName);
			}
			break;
		case IMessage::Type::MUCMessage:
			isHighlightMsg = Core::Instance ().IsHighlightMessage (msg);
			if (isHighlightMsg && XmlSettingsManager::Instance ().property ("NotifyAboutConferenceHighlights").toBool ())
			{
				if (showMsg)
				{
					title = parentCL->GetEntryName ();
					msgString = fmtEntryName + ": " + body;
				}
				else
					msgString = tr ("%1 highlighted you in %2.")
							.arg (fmtEntryName, Util::FormatName (parentCL->GetEntryName ()));
			}
			break;
		default:
			return;
		}

		auto e = Util::MakeNotification (title, msgString, Priority::Info);

		if (msgString.isEmpty ())
			e.Mime_ += "+advanced";

		auto entry = msg->GetMessageType () == IMessage::Type::MUCMessage ?
				parentCL :
				other;

		const auto count = ++UnreadCounts_ [entry];
		if (msg->GetMessageType () == IMessage::Type::MUCMessage)
		{
			e.Additional_ [Fields::SubSourceID] = other->GetEntryID ();
			e.Additional_ [AN::EF::EventType] = isHighlightMsg ? AN::TypeIMMUCHighlight : AN::TypeIMMUCMsg;

			const auto& mucName = Util::FormatName (parentCL->GetEntryName ());
			const auto notifMsg = isHighlightMsg ?
					tr ("%n message(s) from %1 in %2", nullptr, count).arg (fmtEntryName, mucName) :
					tr ("%n message(s) in %1", nullptr, count).arg (mucName);
			e.Additional_ [AN::EF::FullText] = notifMsg;
		}
		else
		{
			e.Additional_ [AN::EF::EventType] = AN::TypeIMIncMsg;
			e.Additional_ [AN::EF::FullText] = tr ("%n message(s) from %1", nullptr, count).arg (fmtEntryName);
		}

		e.Additional_ [AN::EF::Count] = count;
		e.Additional_ [AN::EF::ExtendedText] = tr ("%n message(s)", nullptr, count);
		e.Additional_ [Fields::Msg] = body;

		const auto nh = new Util::NotificationActionHandler { e, this };
		nh->AddFunction (tr ("Open chat"),
				[parentCL] { Core::Instance ().GetChatTabsManager ()->OpenChat (*parentCL, true); });
		nh->AddDependentObject (parentCL->GetQObject ());

		Util::Sequence (this, BuildNotification (AvatarsMgr_, e, entry, {}, other)) >>
				[this] (const Entity& e) { EntityMgr_->HandleEntity (e); };
	}

	namespace
	{
		QString GetStatusChangeText (const ICLEntry *entry,
				const EntryStatus& entrySt, const QString& variant, QString status)
		{
			const auto& statusString = entrySt.StatusString_.toHtmlEscaped ();
			if (!statusString.isEmpty ())
				status += " (" + statusString + ")";

			const auto& name = entry->GetEntryName ();
			const auto parent = entry->GetParentCLEntry ();
			if (!variant.isEmpty () || !parent)
				return NotificationsManager::tr ("%1 is now %2.")
						.arg (Util::FormatName (variant.isEmpty () ? name : name + '/' + variant), status);
			return NotificationsManager::tr ("%1 in room %2 is now %3.")
					.arg (Util::FormatName (name), Util::FormatName (parent->GetEntryName ()), status);
		}
	}

	void NotificationsManager::HandleStatusChanged (ICLEntry *entry,
			const EntryStatus& entrySt, const QString& variant)
	{
		const auto acc = entry->GetParentAccount ();
		if (!LastAccountStatusChange_.contains (acc) ||
				LastAccountStatusChange_ [acc].secsTo (QDateTime::currentDateTime ()) < 5)
			return;

		const auto extAcc = qobject_cast<IExtSelfInfoAccount*> (entry->GetParentAccount ()->GetQObject ());
		if (extAcc &&
				extAcc->GetSelfContact () == entry->GetQObject ())
			return;

		const auto& text = GetStatusChangeText (entry, entrySt, variant, StateToString (entrySt.State_));

		auto e = Util::MakeNotification (EventTitle, text, Priority::Info);
		e.Mime_ += "+advanced";

		e.Additional_ [AN::EF::EventType] = AN::TypeIMStatusChange;
		e.Additional_ [AN::EF::FullText] = text;
		e.Additional_ [AN::EF::ExtendedText] = text;
		e.Additional_ [AN::EF::Count] = 1;
		e.Additional_ [Fields::Msg] = entrySt.StatusString_;
		e.Additional_ [Fields::NewStatus] = StateToID (entrySt.State_);

		Util::Sequence (this, BuildNotification (AvatarsMgr_, e, entry, "StatusChangeEvent")) >>
				[this] (const Entity& e) { EntityMgr_->HandleEntity (e); };
	}


	void NotificationsManager::handleClearUnreadMsgCount (QObject *entryObj)
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		UnreadCounts_.remove (entry);

		const auto& entryID = entry->GetEntryID ();

		EntityMgr_->HandleEntity (Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.IncomingMessageFrom/" + entryID));
		EntityMgr_->HandleEntity (Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.AttentionDrawnBy/" + entryID));
	}

	namespace
	{
		QString GetTuneHRText (ICLEntry *entry, const Media::AudioInfo& info)
		{
			const auto& entryName = entry->GetEntryName ();
			return !info.Title_.isEmpty () ?
					NotificationsManager::tr ("%1 is now listening to %2 by %3.")
							.arg (Util::FormatName (entryName), Util::FormatName (info.Title_), Util::FormatName (info.Artist_)) :
					NotificationsManager::tr ("%1 stopped listening to music.")
							.arg (Util::FormatName (entryName));
		}
	}

	void NotificationsManager::handleTuneChanged (const QString& variant)
	{
		const auto entry = qobject_cast<ICLEntry*> (sender ());

		const auto& info = qobject_cast<IHaveContactTune*> (sender ())->GetUserTune (variant);
		const auto& text = GetTuneHRText (entry, info);

		auto e = Util::MakeNotification (EventTitle, text, Priority::Info);
		e.Mime_ += "+advanced";

		e.Additional_ [AN::EF::EventType] = AN::TypeIMEventTuneChange;

		e.Additional_ [AN::EF::FullText] = text;
		e.Additional_ [AN::EF::ExtendedText] = text;
		e.Additional_ [AN::EF::Count] = 1;

		e.Additional_ [AN::Field::MediaArtist] = info.Artist_;
		e.Additional_ [AN::Field::MediaAlbum] = info.Album_;
		e.Additional_ [AN::Field::MediaPlayerURL] = info.Other_ ["URL"];
		e.Additional_ [AN::Field::MediaTitle] = info.Title_;
		e.Additional_ [AN::Field::MediaLength] = info.Length_;

		Util::Sequence (this, BuildNotification (AvatarsMgr_, e, entry, "TuneChangeEvent")) >>
				[this] (const Entity& e) { EntityMgr_->HandleEntity (e); };
	}

	namespace
	{
		QString GetActivityHRText (ICLEntry *entry, const ActivityInfo& info)
		{
			const auto& entryName = entry->GetEntryName ();
			if (info.General_.isEmpty ())
				return NotificationsManager::tr ("%1 is not doing anything anymore.")
						.arg (Util::FormatName (entryName));

			if (info.Specific_.isEmpty ())
				return NotificationsManager::tr ("%1 is now %2.")
						.arg (Util::FormatName (entryName), ActivityDialog::ToHumanReadable (info.General_));

			return NotificationsManager::tr ("%1 is now %2 (in particular, %3).")
					.arg (Util::FormatName (entryName),
							ActivityDialog::ToHumanReadable (info.General_),
							ActivityDialog::ToHumanReadable (info.Specific_));
		}
	}

	void NotificationsManager::handleActivityChanged (const QString& variant)
	{
		const auto entry = qobject_cast<ICLEntry*> (sender ());
		const auto ihca = qobject_cast<IHaveContactActivity*> (sender ());
		const auto& info = ihca->GetUserActivity (variant);
		const auto& text = GetActivityHRText (entry, info);

		auto e = Util::MakeNotification (EventTitle, text, Priority::Info);
		e.Mime_ += "+advanced";

		e.Additional_ [AN::EF::EventType] = AN::TypeIMEventActivityChange;

		e.Additional_ [AN::EF::FullText] = text;
		e.Additional_ [AN::EF::ExtendedText] = text;
		e.Additional_ [AN::EF::Count] = 1;

		e.Additional_ [AN::Field::IMActivityGeneral] = info.General_;
		e.Additional_ [AN::Field::IMActivitySpecific] = info.Specific_;
		e.Additional_ [AN::Field::IMActivityText] = info.Text_;

		Util::Sequence (this, BuildNotification (AvatarsMgr_, e, entry, "ActivityChangeEvent")) >>
				[this] (const Entity& e) { EntityMgr_->HandleEntity (e); };
	}

	namespace
	{
		QString GetMoodHRText (ICLEntry *entry, const MoodInfo& info)
		{
			const auto& entryName = entry->GetEntryName ();
			if (info.Mood_.isEmpty ())
				return NotificationsManager::tr ("%1 is not in any particular mood anymore.")
						.arg (Util::FormatName (entryName));

			return NotificationsManager::tr ("%1 is now %2.")
					.arg (Util::FormatName (entryName), MoodDialog::ToHumanReadable (info.Mood_));
		}
	}

	void NotificationsManager::handleMoodChanged (const QString& variant)
	{
		const auto entry = qobject_cast<ICLEntry*> (sender ());
		const auto& info = qobject_cast<IHaveContactMood*> (sender ())->GetUserMood (variant);
		const auto& text = GetMoodHRText (entry, info);

		auto e = Util::MakeNotification (EventTitle, text, Priority::Info);
		e.Mime_ += "+advanced";

		e.Additional_ [AN::EF::EventType] = AN::TypeIMEventMoodChange;

		e.Additional_ [AN::EF::FullText] = text;
		e.Additional_ [AN::EF::ExtendedText] = text;
		e.Additional_ [AN::EF::Count] = 1;

		e.Additional_ [AN::Field::IMMoodGeneral] = info.Mood_;
		e.Additional_ [AN::Field::IMMoodText] = info.Text_;

		Util::Sequence (this, BuildNotification (AvatarsMgr_, e, entry, "MoodChangeEvent")) >>
				[this] (const Entity& e) { EntityMgr_->HandleEntity (e); };
	}

	namespace
	{
		struct GeolocationInfo
		{
			bool IsValid_;
			double Lon_;
			double Lat_;

			QString Country_;
			QString Locality_;
		};

		QString GetHRLocationText (ICLEntry *entry, const GeolocationInfo& info)
		{
			const auto& entryName = entry->GetEntryName ();
			if (!info.IsValid_)
				return NotificationsManager::tr ("%1's location is not known.")
						.arg (Util::FormatName (entryName));

			const bool hasCountry = !info.Country_.isEmpty ();
			const bool hasLocality = !info.Locality_.isEmpty ();
			if (hasCountry && hasLocality)
				return NotificationsManager::tr ("%1 is now in %2 (%3).")
						.arg (Util::FormatName (entryName), info.Locality_, info.Country_);

			if (hasCountry || hasLocality)
				return NotificationsManager::tr ("%1 is now in %2.")
						.arg (Util::FormatName (entryName), hasCountry ? info.Country_ : info.Locality_);

			return NotificationsManager::tr ("%1's location updated.")
					.arg (Util::FormatName (entryName));
		}
	}

	void NotificationsManager::handleLocationChanged (const QString& variant)
	{
		const auto entry = qobject_cast<ICLEntry*> (sender ());
		const auto acc = entry->GetParentAccount ();
		const auto isg = qobject_cast<ISupportGeolocation*> (acc->GetQObject ());
		if (!isg)
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< acc->GetQObject ()
					<< "does not implement ISupportGeolocation";
			return;
		}

		const auto& infoMap = isg->GetUserGeolocationInfo (sender (), variant);
		const auto& info = [&infoMap]
			{
				bool lonOk = false;
				bool latOk = false;

				GeolocationInfo info
				{
					true,
					infoMap ["lon"].toDouble (&lonOk),
					infoMap ["lat"].toDouble (&latOk),
					infoMap ["country"].toString (),
					infoMap ["locality"].toString ()
				};
				info.IsValid_ = lonOk && latOk;
				return info;
			} ();

		const auto& text = GetHRLocationText (entry, info);

		auto e = Util::MakeNotification (EventTitle, text, Priority::Info);
		e.Mime_ += "+advanced";

		e.Additional_ [AN::EF::EventType] = AN::TypeIMEventLocationChange;

		e.Additional_ [AN::EF::FullText] = text;
		e.Additional_ [AN::EF::ExtendedText] = text;
		e.Additional_ [AN::EF::Count] = 1;

		e.Additional_ [AN::Field::IMLocationLongitude] = info.Lon_;
		e.Additional_ [AN::Field::IMLocationLatitude] = info.Lat_;

		Util::Sequence (this, BuildNotification (AvatarsMgr_, e, entry, "LocationChangeEvent")) >>
				[this] (const Entity& e) { EntityMgr_->HandleEntity (e); };
	}

	void NotificationsManager::handleAttentionDrawn (const QString& text, const QString&)
	{
		if (!XmlSettingsManager::Instance ().property ("RespectDrawAttentions").toBool ())
			return;

		auto& entry = qobject_ref_cast<ICLEntry> (*sender ());

		const auto& name = Util::FormatName (entry.GetEntryName ());
		const auto& str = text.isEmpty () ?
				tr ("%1 requests your attention.").arg (name) :
				tr ("%1 requests your attention: %2.").arg (name, text);

		auto e = Util::MakeNotification (EventTitle, str, Priority::Info);
		e.Additional_ [AN::EF::DeltaCount] = 1;
		e.Additional_ [AN::EF::EventType] = AN::TypeIMAttention;
		e.Additional_ [AN::EF::ExtendedText] = tr ("Attention requested.");
		e.Additional_ [AN::EF::FullText] = tr ("Attention requested by %1.").arg (entry.GetEntryName ());
		e.Additional_ [Fields::Msg] = text;

		const auto nh = new Util::NotificationActionHandler { e };
		nh->AddFunction (tr ("Open chat"),
				[&entry] { Core::Instance ().GetChatTabsManager ()->OpenChat (entry, true); });
		nh->AddDependentObject (entry.GetQObject ());

		Util::Sequence (this, BuildNotification (AvatarsMgr_, e, &entry, "AttentionDrawnBy")) >>
				[this] (const Entity& e) { EntityMgr_->HandleEntity (e); };
	}

	void NotificationsManager::HandleChatPartStateChanged (ICLEntry *entry, ChatPartState state, const QString&)
	{
		if (state != CPSComposing)
			return;

		const auto& id = entry->GetEntryID ();
		if (!ShouldNotifyNextTyping_.value (id, true))
			return;

		const auto& type = XmlSettingsManager::Instance ()
				.property ("NotifyIncomingComposing").toString ();
		if (type != "all" &&
			!(type == "opened" &&
				Core::Instance ().GetChatTabsManager ()->IsOpenedChat (id)))
			return;

		ShouldNotifyNextTyping_ [id] = false;

		auto e = Util::MakeNotification (EventTitle,
				tr ("%1 started composing a message to you.")
					.arg (Util::FormatName (entry->GetEntryName ())),
				Priority::Info);

		const auto nh = new Util::NotificationActionHandler { e };
		nh->AddFunction (tr ("Open chat"),
				[entry] { Core::Instance ().GetChatTabsManager ()->OpenChat (*entry, true); });
		nh->AddDependentObject (entry->GetQObject ());

		Util::Sequence (this, BuildNotification (AvatarsMgr_, e, entry, "Typing")) >>
				[this] (const Entity& e) { EntityMgr_->HandleEntity (e); };
	}

	void NotificationsManager::handleEntryMadeCurrent (QObject *entryObj)
	{
		if (entryObj)
			ShouldNotifyNextTyping_ [qobject_ref_cast<ICLEntry> (entryObj).GetEntryID ()] = true;
	}
}
}
