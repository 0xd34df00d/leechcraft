/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chateventsadapter.h"
#include <QCoreApplication>
#include <util/gui/util.h>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include "util/azoth/util.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucentry.h"
#include "interfaces/azoth/imucperms.h"
#include "../../xmlsettingsmanager.h"

namespace LC::Azoth
{
	namespace
	{
		bool CheckShow (const char *prop)
		{
			return XmlSettingsManager::Instance ().property (prop).toBool ();
		}

		QString GetDisplayId (const ICLEntry& entry, const QString& variant)
		{
			return variant.isEmpty () ? entry.GetEntryName () : entry.GetEntryName () + '/' + variant;
		}

		MUCPerms_t SubtractPerms (const MUCPerms_t& minuend, const MUCPerms_t& subtrahend)
		{
			MUCPerms_t result;
			for (const auto& [key, minuendList] : minuend.asKeyValueRange ())
			{
				const auto& subtrahendList = subtrahend.value (key);
				for (const auto& item : minuendList)
					if (!subtrahendList.contains (item))
						result [key] << item;
			}
			return result;
		}

		auto ComputePermsDiff (const MUCPerms_t& oldPerms, const MUCPerms_t& newPerms)
		{
			struct PermsDiff
			{
				MUCPerms_t Removed_;
				MUCPerms_t Added_;
			};
			return PermsDiff
			{
				.Removed_ = SubtractPerms (oldPerms, newPerms),
				.Added_ = SubtractPerms (newPerms, oldPerms),
			};
		}

		class DirectAdapter : public ChatEventsAdapter
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Azoth::DirectAdapter)

			ICLEntry& Entry_;
		public:
			DirectAdapter (ICLEntry& entry, QObject *parent)
			: ChatEventsAdapter { parent }
			, Entry_ { entry }
			{
				auto& emitter = Entry_.GetCLEntryEmitter ();
				connect (&emitter,
						&Emitters::CLEntry::chatPartStateChanged,
						this,
						&DirectAdapter::HandleChatState);
				connect (&emitter,
						&Emitters::CLEntry::statusChanged,
						this,
						&DirectAdapter::EmitStatusEvent);
			}
		private:
			void HandleChatState (ChatPartState state, const QString& variant)
			{
				if (state != CPSGone || !CheckShow ("ShowEndConversations"))
					return;

				emit gotEvent ({ .Text_ { tr ("%1 has left the conversation").arg (GetDisplayId (Entry_, variant)) }  });
			}

			void EmitStatusEvent (const EntryStatus& status, const QString& variant)
			{
				if (CheckShow ("ShowStatusChangesEventsInPrivates"))
					EmitStatusEventImpl (GetDisplayId (Entry_, variant), status);
			}
		};

		QStringList PermsToStrings (const IMUCPerms& perms, const MUCPerms_t& permsMap)
		{
			QStringList permsStrings;
			for (const auto& permsList : permsMap)
				for (const auto& perm : permsList)
					permsStrings << perms.GetUserString (perm);
			return permsStrings;
		}

		class MucAdapter : public ChatEventsAdapter
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Azoth::MucAdapter)

			IMUCEntry& Entry_;
			IMUCPerms * const Perms_;
		public:
			MucAdapter (ICLEntry& entry, IMUCEntry& mucEntry, QObject *parent)
			: ChatEventsAdapter { parent }
			, Entry_ { mucEntry }
			, Perms_ { qobject_cast<IMUCPerms*> (entry.GetQObject ()) }
			{
				auto& emitter = Entry_.GetMUCEntryEmitter ();

				const auto setupParticipant = [this] (ICLEntry& part)
				{
					auto& partEmitter = part.GetCLEntryEmitter ();
					connect (&partEmitter,
							&Emitters::CLEntry::statusChanged,
							this,
							std::bind_front (&MucAdapter::EmitStatusEvent, this, std::ref (part)));
				};

				for (const auto part : Entry_.GetParticipants ())
					setupParticipant (*part);

				connect (&emitter,
						&Emitters::MUCEntry::participantJoined,
						this,
						[this, setupParticipant] (ICLEntry& part, MucEvents::Liveness liveness)
						{
							setupParticipant (part);
							if (liveness == MucEvents::Liveness::Live)
								EmitJoinEvent (part);
						});
				connect (&emitter,
						&Emitters::MUCEntry::participantLeaving,
						this,
						[this] (ICLEntry& part, const MucEvents::ParticipantLeaveInfo& info)
						{
							part.GetCLEntryEmitter ().disconnect (this);
							EmitLeaveEvent (part, info);
						});
				connect (&emitter,
						&Emitters::MUCEntry::mucSubjectChanged,
						this,
						[this] (const MucEvents::SubjectChange& event)
						{
							const auto& text = event.ActorNick_ ?
									tr ("%1 changed subject to %2").arg (*event.ActorNick_, event.Subject_) :
									tr ("Subject changed to %1").arg (event.Subject_);
							emit gotEvent ({ .Text_ { text } });
						});

				if (Perms_)
				{
					auto& permsEmitter = Perms_->GetMUCPermsEmitter ();
					connect (&permsEmitter,
							&Emitters::MUCPerms::permsChanged,
							this,
							[this] (const MucEvents::ParticipantPermsChange& event)
							{
								if (const auto& text = MakePermsChangeString (event);
									!text.isEmpty ())
									emit gotEvent ({ .Text_ { text } });
							});
				}
			}
		private:
			static QString MakeJoinString (const ICLEntry& part, const QStringList& perms)
			{
				const auto& name = part.GetEntryName ();
				return perms.isEmpty () ?
						tr ("%1 has entered the room").arg (name) :
						tr ("%1 has entered the room as %2").arg (name, Util::FormatHumanReadableList (perms));
			}

			static QString MakeActorReasonClause (const std::optional<QString>& actor, const QString& reason)
			{
				const bool hasActor = actor.has_value ();
				const bool hasReason = !reason.isEmpty ();

				if (hasActor && hasReason)
					//: Parenthetical attribution in MUC events: %1 performed the action, %2 is the reason.
					return tr ("by %1: %2").arg (*actor, reason);
				if (hasActor)
					//: Parenthetical attribution in MUC events: %1 performed the action.
					return tr ("by %1").arg (*actor);
				if (hasReason)
					//: Parenthetical attribution in MUC events: %1 is the reason for the action.
					return tr ("reason: %1").arg (reason);
				return {};
			}

			QString MakePermsDiffClause (const MUCPerms_t& oldPerms, const MUCPerms_t& newPerms)
			{
				const auto& diff = ComputePermsDiff (oldPerms, newPerms);
				const auto& removedString = Util::FormatHumanReadableList (PermsToStrings (*Perms_, diff.Removed_));
				const auto& addedString = Util::FormatHumanReadableList (PermsToStrings (*Perms_, diff.Added_));
				const auto hasRemoved = !diff.Removed_.isEmpty ();
				const auto hasAdded = !diff.Added_.isEmpty ();
				if (hasRemoved && hasAdded)
					//: Perms delta when participant both gained and lost perms; %1 is the added list, %2 the lost list
					return tr ("added %1, no longer %2").arg (addedString, removedString);
				if (hasAdded)
					//: Perms delta, gained-only; %1 is the list of newly granted perms.
					return tr ("added %1").arg (addedString);
				if (hasRemoved)
					//: Perms delta, lost-only; %1 is the list of revoked perms.
					return tr ("no longer %1").arg (removedString);
				return {};
			}

			QString MakePermsChangeString (const MucEvents::ParticipantPermsChange& event)
			{
				const auto& name = event.Participant_.GetEntryName ();
				const auto& actorReasonAddendum = MakeActorReasonClause (event.ActorNick_, event.Reason_);

				const auto& currPerms = Perms_->GetPerms (event.Participant_);
				if (currPerms.isEmpty ())
				{
					if (!event.PrevPerms_)
						return actorReasonAddendum.isEmpty () ?
								tr ("%1 now has no permissions").arg (name) :
								tr ("%1 now has no permissions (%2)").arg (name, actorReasonAddendum);

					const auto& prevPermsString = Util::FormatHumanReadableList (PermsToStrings (*Perms_, *event.PrevPerms_));
					return actorReasonAddendum.isEmpty () ?
							tr ("%1 now has no permissions; had %2").arg (name, prevPermsString) :
							tr ("%1 now has no permissions; had %2 (%3)").arg (name, prevPermsString, actorReasonAddendum);
				}

				const auto& currPermsString = Util::FormatHumanReadableList (PermsToStrings (*Perms_, currPerms));
				const auto& permsDiffClause = event.PrevPerms_ ?
						MakePermsDiffClause (*event.PrevPerms_, currPerms) :
						QString {};
				if (permsDiffClause.isEmpty ())
					return actorReasonAddendum.isEmpty () ?
							tr ("%1 is now %2").arg (name, currPermsString) :
							tr ("%1 is now %2 (%3)").arg (name, currPermsString, actorReasonAddendum);
				return actorReasonAddendum.isEmpty () ?
						tr ("%1 is now %2: %3").arg (name, currPermsString, permsDiffClause) :
						tr ("%1 is now %2: %3 (%4)").arg (name, currPermsString, permsDiffClause, actorReasonAddendum);
			}

			void EmitJoinEvent (const ICLEntry& part)
			{
				if (!CheckShow ("ShowJoinsLeaves"))
					return;

				QStringList permsStrings;
				if (Perms_)
					permsStrings = PermsToStrings (*Perms_, Perms_->GetPerms (part));
				emit gotEvent ({ .Text_ { MakeJoinString (part, permsStrings) } });
			}

			static QString GetOutActionString (MucEvents::ParticipantForcedOut::Action action)
			{
				switch (action)
				{
				case MucEvents::ParticipantForcedOut::Action::Kicked:
					return tr ("kicked");
				case MucEvents::ParticipantForcedOut::Action::Banned:
					return tr ("banned");
				}
				std::unreachable ();
			}

			static QString MakeLeaveString (const ICLEntry& part, const MucEvents::ParticipantLeaveInfo& info)
			{
				const auto& name = part.GetEntryName ();
				return Util::Visit (info,
						[&name] (const MucEvents::ParticipantLeft& event)
						{
							if (!CheckShow ("ShowJoinsLeaves"))
								return QString {};

							if (event.Message_.isEmpty ())
								return tr ("%1 has left the room").arg (name);
							return tr ("%1 has left the room: %2").arg (name, event.Message_);
						},
						[&name] (const MucEvents::ParticipantForcedOut& event)
						{
							const auto& action = GetOutActionString (event.Action_);
							const auto actorNick = event.Actor_ ? event.Actor_->GetEntryName () : std::optional<QString> {};
							const auto& addendum = MakeActorReasonClause (actorNick, event.Reason_);
							return addendum.isEmpty () ?
									tr ("%1 has been %2").arg (name, action) :
									tr ("%1 has been %2 (%3)").arg (name, action, addendum);
						});
			}

			void EmitLeaveEvent (const ICLEntry& part, const MucEvents::ParticipantLeaveInfo& info)
			{
				if (const auto& str = MakeLeaveString (part, info);
					!str.isEmpty ())
					emit gotEvent ({ .Text_ { str } });
			}

			void EmitStatusEvent (const ICLEntry& part, const EntryStatus& status)
			{
				if (CheckShow ("ShowStatusChangesEvents"))
					EmitStatusEventImpl (part.GetEntryName (), status);
			}
		};
	}

	ChatEventsAdapter* ChatEventsAdapter::MakeFor (ICLEntry& entry, QObject *parent)
	{
		switch (entry.GetEntryType ())
		{
		case ICLEntry::EntryType::Chat:
		case ICLEntry::EntryType::PrivateChat:
		case ICLEntry::EntryType::UnauthEntry:
			return new DirectAdapter { entry, parent };
		case ICLEntry::EntryType::MUC:
			return new MucAdapter { entry, qobject_ref_cast<IMUCEntry> (entry.GetQObject ()), parent };
		}

		qFatal () << "unknown entry type:" << static_cast<int> (entry.GetEntryType ());
	}

	void ChatEventsAdapter::EmitStatusEventImpl (const QString& displayName, const EntryStatus& status)
	{
		const auto& stateStr = StateToString (status.State_);
		const auto& statusText = status.StatusString_.isEmpty () ?
				stateStr :
				"%1 (%2)"_qs.arg (stateStr, status.StatusString_);
		emit gotEvent ({ .Text_ { tr ("%1 is now %2").arg (displayName, statusText) } });
	}
}
