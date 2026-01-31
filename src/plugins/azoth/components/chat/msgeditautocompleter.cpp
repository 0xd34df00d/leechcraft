/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "msgeditautocompleter.h"
#include <QKeyEvent>
#include <QTextEdit>
#include <util/sll/visitor.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/azoth/iprovidecommands.h>
#include "components/util/entries.h"
#include "../../core.h"
#include "../../xmlsettingsmanager.h"
#include "../../corecommandsmanager.h"

namespace LC::Azoth
{
	MsgEditAutocompleter::MsgEditAutocompleter (const QString& entryId, QTextEdit& edit)
	: QObject { &edit }
	, EntryId_ { entryId }
	, Edit_ { edit }
	{
		Edit_.installEventFilter (this);

		const auto reset = [this]
		{
			if (!InsertingCompletion_)
				State_ = Idle {};
		};
		connect (&edit,
				&QTextEdit::textChanged,
				this,
				reset);
		connect (&edit,
				&QTextEdit::cursorPositionChanged,
				this,
				reset);
	}

	bool MsgEditAutocompleter::eventFilter (QObject *, QEvent *event)
	{
		if (event->type () != QEvent::KeyPress)
			return false;

		const auto& kev = dynamic_cast<const QKeyEvent&> (*event);
		const auto isForwardTab = kev.key () == Qt::Key_Tab && kev.modifiers () == Qt::NoModifier;
		const auto isBackwardTab = kev.key () == Qt::Key_Backtab ||
				(kev.key () == Qt::Key_Tab && kev.modifiers () == Qt::ShiftModifier);
		if (isForwardTab)
		{
			Complete (Direction::Forward);
			return true;
		}
		if (isBackwardTab)
		{
			Complete (Direction::Backward);
			return true;
		}

		return false;
	}

	namespace
	{
		struct CompTrigger
		{
			QString Word_;
			bool IsStartOfLine_;
			bool IsStartOfMessage_;
			int StartPos_;

			auto ToWordRemovalPred () const
			{
				return [prefix = Word_] (const QString& str) { return !str.startsWith (prefix, Qt::CaseInsensitive); };
			}
		};

		std::optional<CompTrigger> GetCompTrigger (QTextEdit& edit)
		{
			const auto& text = edit.toPlainText ();
			const auto pos = edit.textCursor ().position ();

			const auto rightIt = std::find_if (text.begin () + pos, text.end (),
					[] (QChar ch) { return ch.isSpace (); });
			const auto leftIt = std::find_if (QString::const_reverse_iterator { text.begin () + pos }, text.rend (),
					[] (QChar ch) { return ch.isSpace (); }).base ();
			const int rightPos = rightIt - text.begin ();
			const int leftPos = leftIt - text.begin ();
			const auto& word = text.mid (leftPos, rightPos - leftPos);
			const bool isStartOfLine = !leftPos || text [leftPos - 1] == '\n';
			return CompTrigger { .Word_ = word, .IsStartOfLine_ = isStartOfLine, .IsStartOfMessage_ = !leftPos, .StartPos_ = leftPos };
		}

		QStringList GetCommandCompletions (ICLEntry& entry, const CompTrigger& trigger)
		{
			if (!trigger.IsStartOfMessage_)
				return {};

			QStringList result;
			auto cmdProvs = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IProvideCommands*> ();
			cmdProvs << Core::Instance ().GetCoreCommandsManager ();
			for (const auto prov : cmdProvs)
				for (const auto& cmd : prov->GetStaticCommands (&entry))
					result += cmd.Names_;
			result.removeIf (trigger.ToWordRemovalPred ());
			return result;
		}

		QStringList GetNickCompletions (ICLEntry& entry, const CompTrigger& trigger)
		{
			const auto mucEntry = qobject_cast<IMUCEntry*> (entry.GetQObject ());
			if (!mucEntry)
				return {};

			auto nicks = GetMucParticipants (entry.GetEntryID ());
			nicks.removeAll (mucEntry->GetNick ());
			nicks.removeIf (trigger.ToWordRemovalPred ());
			std::ranges::sort (nicks,
					[] (const QString& l, const QString& r)
					{
						return QString::localeAwareCompare (l.toLower (), r.toLower ()) < 0;
					});

			QString postNick;
			if (trigger.IsStartOfLine_)
				postNick = XmlSettingsManager::Instance ().property ("PostAddressText").toString ();
			postNick += ' ';

			for (auto& nick : nicks)
				nick += postNick;

			return nicks;
		}

		QStringList GetCompletions (const CompTrigger& trigger, ICLEntry& entry)
		{
			return GetNickCompletions (entry, trigger) + GetCommandCompletions (entry, trigger);
		}
	}

	void MsgEditAutocompleter::Complete (Direction dir)
	{
		const auto entry = qobject_cast<ICLEntry*> (Core::Instance ().GetEntry (EntryId_));
		if (!entry)
			return;

		InsertingCompletion_ = true;
		const auto guard = Util::MakeScopeGuard ([this] { InsertingCompletion_ = false; });
		State_ = Util::Visit (State_,
				[this, entry, dir] (Idle) -> State
				{
					const auto& trigger = GetCompTrigger (Edit_);
					if (!trigger)
						return NoCompletions {};
					const auto& comps = GetCompletions (*trigger, *entry);
					if (comps.isEmpty ())
						return NoCompletions {};
					return OfferCompletion (Completing { trigger->StartPos_, comps, -DirectionDelta (dir), trigger->Word_.size () }, dir);
				},
				[this, dir] (const Completing& comp) { return State { OfferCompletion (comp, dir) }; },
				[] (NoCompletions) { return State { NoCompletions {} }; });
	}

	MsgEditAutocompleter::Completing MsgEditAutocompleter::OfferCompletion (Completing comp, Direction dir)
	{
		auto c = Edit_.textCursor ();
		c.setPosition (comp.StartCursorPos_);
		c.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, comp.CurCompLength_);

		comp.Idx_ += DirectionDelta (dir);
		if (comp.Idx_ < 0)
			comp.Idx_ += comp.Completions_.size ();
		comp.Idx_ %= comp.Completions_.size ();

		const auto& nextComp = comp.Completions_ [comp.Idx_];
		c.insertText (nextComp);

		comp.CurCompLength_ = nextComp.size ();

		return comp;
	}

	int MsgEditAutocompleter::DirectionDelta (Direction dir)
	{
		switch (dir)
		{
		case Direction::Forward:
			return +1;
		case Direction::Backward:
			return -1;
		}

		std::unreachable ();
	}
}
