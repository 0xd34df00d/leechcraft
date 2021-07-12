/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "abbrevsmanager.h"
#include <QSettings>
#include <QCoreApplication>
#include <interfaces/azoth/iprovidecommands.h>

namespace LC::Azoth::Abbrev
{
	AbbrevsManager::AbbrevsManager (QObject *parent)
	: QObject { parent }
	{
		Load ();
	}

	void AbbrevsManager::Add (const Abbreviation& abbrev)
	{
		if (std::any_of (Abbrevs_.begin (), Abbrevs_.end (),
				[&abbrev] (const Abbreviation& other)
					{ return other.Pattern_ == abbrev.Pattern_; }))
			throw CommandException { tr ("Abbreviation with this pattern already exists.") };

		if (abbrev.Pattern_.isEmpty ())
			throw CommandException { tr ("Abbeviation pattern is empty.") };

		if (abbrev.Expansion_.isEmpty ())
			throw CommandException { tr ("Abbeviation expansion is empty.") };

		const auto pos = std::lower_bound (Abbrevs_.begin (), Abbrevs_.end (), abbrev,
				[] (const Abbreviation& left, const Abbreviation& right)
					{ return left.Pattern_.size () > right.Pattern_.size (); });
		Abbrevs_.insert (pos, abbrev);

		Save ();
	}

	const QList<Abbreviation>& AbbrevsManager::List () const
	{
		return Abbrevs_;
	}

	void AbbrevsManager::Remove (int index)
	{
		if (index < 0 || index >= Abbrevs_.size ())
			return;

		Abbrevs_.removeAt (index);
		Save ();
	}

	namespace
	{
		bool IsBadChar (QChar c)
		{
			return c.isLetter ();
		}

		bool TryExpand (QString& result, const Abbreviation& abbrev)
		{
			if (!result.contains (abbrev.Pattern_))
				return false;

			bool changed = false;

			auto pos = 0;
			while ((pos = result.indexOf (abbrev.Pattern_, pos)) != -1)
			{
				const auto afterAbbrevPos = pos + abbrev.Pattern_.size ();
				if ((!pos || !IsBadChar (result.at (pos - 1))) &&
						(afterAbbrevPos >= result.size () || !IsBadChar (result.at (afterAbbrevPos))))
				{
					result.replace (pos, abbrev.Pattern_.size (), abbrev.Expansion_);
					pos += abbrev.Expansion_.size ();
					changed = true;
				}
				else
					pos += abbrev.Pattern_.size ();
			}

			return changed;
		}
	}

	QString AbbrevsManager::Process (QString text) const
	{
		int cyclesCount = 0;
		while (true)
		{
			bool changed = false;
			for (const auto& abbrev : Abbrevs_)
				if (TryExpand (text, abbrev))
					changed = true;

			if (!changed)
				break;

			const auto expansionsLimit = 1024;
			if (++cyclesCount >= expansionsLimit)
				throw CommandException { tr ("Too many expansions during abbreviations application. Check your rules.") };
		}

		return text;
	}

	namespace Keys
	{
		const QString Group = QStringLiteral ("Abbrevs");
		const QString Abbreviations = QStringLiteral ("Abbreviations");
	}

	void AbbrevsManager::Load ()
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Abbrev" };
		settings.beginGroup (Keys::Group);
		Abbrevs_ = settings.value (Keys::Abbreviations).value<decltype (Abbrevs_)> ();
		settings.endGroup ();
	}

	void AbbrevsManager::Save () const
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Abbrev" };
		settings.beginGroup (Keys::Group);
		settings.setValue (Keys::Abbreviations, QVariant::fromValue (Abbrevs_));
		settings.endGroup ();
	}
}
