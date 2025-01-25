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
#include <util/sys/encodingconverter.h>
#include "knowndictsmanager.h"

Q_DECLARE_METATYPE (QSet<QString>)

namespace LC
{
namespace Rosenthal
{
	struct Checker::HunspellItem
	{
		std::unique_ptr<Hunspell> Hunspell_;
		std::unique_ptr<Util::EncodingConverter> Codec_;

		explicit HunspellItem (const QByteArray& path)
		: Hunspell_ { std::make_unique<Hunspell> (path + ".aff", path + ".dic") }
		, Codec_ { std::make_unique<Util::EncodingConverter> (Hunspell_->get_dict_encoding ()) }
		{
		}

		bool Spell (const QString& word) const
		{
			return Hunspell_->spell (Codec_->FromUnicode (word).toStdString ());
		}

		QStringList Suggest (const QString& word) const
		{
			return Util::MapAs<QList> (Hunspell_->suggest (Codec_->FromUnicode (word).toStdString ()),
					[this] (const auto& str) { return Codec_->ToUnicode (str); });
		}
	};

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

	Checker::~Checker () = default;

	QStringList Checker::GetPropositions (const QString& word) const
	{
		QStringList result;
		for (const auto& item : Hunspells_)
			if (!item.Spell (word))
				result += item.Suggest (word);
		return result;
	}

	bool Checker::IsCorrect (const QString& word) const
	{
		if (LearntWords_.contains (word))
			return true;

		return std::ranges::any_of (Hunspells_, [&word] (const HunspellItem& item) { return item.Spell (word); });
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
			try
			{
				Hunspells_.emplace_back (primaryPath.toUtf8 ());
			}
			catch (const Util::EncodingConverter::UnknownEncoding& error)
			{
				qWarning () << "unsupported encoding" << error.GetEncoding () << "for" << language;
			}
		}
	}
}
}
