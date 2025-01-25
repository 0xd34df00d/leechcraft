/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "checker.h"
#include <algorithm>
#include <QFile>
#include <QSettings>
#include <QCoreApplication>
#include <util/util.h>
#include <util/sll/prelude.h>
#include "knowndictsmanager.h"

Q_DECLARE_METATYPE (QSet<QString>)

namespace LC
{
namespace Rosenthal
{
	Checker::Checker (const KnownDictsManager *knownMgr)
	: KnownMgr_ { knownMgr }
	{
		connect (knownMgr,
				SIGNAL (languagesChanged (QStringList)),
				this,
				SLOT (setLanguages (QStringList)));
		setLanguages (knownMgr->GetLanguages ());

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Rosenthal_LearntWords" };
		LearntWords_ = settings.value ("LearntWords").value<QSet<QString>> ();
	}

	namespace
	{
		bool Spell (Hunspell *hs, const QByteArray& word)
		{
			return hs->spell (word.toStdString ());
		}

		QStringList Suggest (Hunspell *hs, const QByteArray& word)
		{
			return Util::MapAs<QList> (hs->suggest (word.toStdString ()), &QString::fromStdString);
		}
	}

	QStringList Checker::GetPropositions (const QString& word) const
	{
		QStringList result;
		for (const auto& item : Hunspells_)
		{
			const auto& encoded = word.toUtf8 ();
			if (!Spell (item.Hunspell_.get (), encoded))
				result += Suggest (item.Hunspell_.get (), encoded);
		}
		return result;
	}

	bool Checker::IsCorrect (const QString& word) const
	{
		if (LearntWords_.contains (word))
			return true;

		return std::any_of (Hunspells_.begin (), Hunspells_.end (),
				[&word] (const HunspellItem& item) { return Spell (item.Hunspell_.get (), word.toUtf8 ()); });
	}

	void Checker::LearnWord (const QString& word)
	{
		LearntWords_ << word;

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Rosenthal_LearntWords" };
		settings.setValue ("LearntWords", QVariant::fromValue (LearntWords_));
	}

	void Checker::setLanguages (const QStringList& languages)
	{
		Hunspells_.clear ();

		for (const auto& language : languages)
		{
			const auto& primaryPath = KnownMgr_->GetDictPath (language);
			HunspellItem item;
			item.Hunspell_ = std::make_unique<Hunspell> ((primaryPath + ".aff").toLatin1 (),
					(primaryPath + ".dic").toLatin1 ());

			if (const auto& encoding = item.Hunspell_->get_dict_encoding ();
				encoding == "UTF-8" || encoding == "ISO8859-1")
				Hunspells_.push_back (std::move (item));
			else
				qWarning () << "unsupported encoding" << encoding;
		}
	}
}
}
