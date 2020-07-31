/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "spellcheckext.h"
#include <QTextBoundaryFinder>
#include <interfaces/core/ipluginsmanager.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace WKPlugins
{
	namespace
	{
		QList<ISpellChecker_ptr> GetSpellcheckers (const ICoreProxy_ptr& proxy)
		{
			const auto ipm = proxy->GetPluginsManager ();

			QList<ISpellChecker_ptr> result;
			for (const auto scProvider : ipm->GetAllCastableTo<ISpellCheckProvider*> ())
				if (const auto sc = scProvider->CreateSpellchecker ())
					result << sc;
			return result;
		}
	}

	QList<ISpellChecker_ptr> SpellcheckerExt::Spellcheckers_ {};

	SpellcheckerExt::SpellcheckerExt (const ICoreProxy_ptr& proxy)
	: Proxy_ { proxy }
	{
		if (Spellcheckers_.empty ())
			Spellcheckers_ = GetSpellcheckers (proxy);
	}

	bool SpellcheckerExt::isContinousSpellCheckingEnabled () const
	{
		return ContinousEnabled_ && !Spellcheckers_.isEmpty ();
	}

	void SpellcheckerExt::toggleContinousSpellChecking ()
	{
		ContinousEnabled_ = !ContinousEnabled_;
	}

	void SpellcheckerExt::learnWord (const QString& word)
	{
		for (const auto& sc : Spellcheckers_)
			sc->LearnWord (word);
	}

	void SpellcheckerExt::ignoreWordInSpellDocument (const QString& word)
	{
		Ignored_ << word;
	}

	namespace
	{
		bool IsWordStart (QTextBoundaryFinder::BoundaryReasons reasons,
				QTextBoundaryFinder::BoundaryType type)
		{
			return reasons & QTextBoundaryFinder::StartOfItem &&
					type & QTextBoundaryFinder::Word;
		}

		bool IsWordEnd (QTextBoundaryFinder::BoundaryReasons reasons,
				QTextBoundaryFinder::BoundaryType type)
		{
			return reasons & QTextBoundaryFinder::EndOfItem &&
					type & QTextBoundaryFinder::Word;
		}
	}

	void SpellcheckerExt::checkSpellingOfString (const QString& word,
			int *misspellingLocation, int *misspellingLength)
	{
		if (!misspellingLength || !misspellingLocation)
			return;

		*misspellingLocation = 0;
		*misspellingLength = 0;

		QTextBoundaryFinder finder { QTextBoundaryFinder::Word, word };
		bool isInWord = false;
		int start = -1;
		int end = -1;
		do
		{
			const auto reasons = finder.boundaryReasons ();

			if (IsWordStart (reasons, finder.type ()))
			{
				start = finder.position ();
				isInWord = true;
			}

			if (isInWord && IsWordEnd (reasons, finder.type ()))
			{
				end = finder.position ();

				const auto& str = word.mid (start, end - start);
				if (std::none_of (Spellcheckers_.begin (), Spellcheckers_.end (),
						[&str] (const ISpellChecker_ptr& sc) { return sc->IsCorrect (str); }))
				{
					*misspellingLocation = start;
					*misspellingLength = end - start;
					return;
				}

				isInWord = false;
			}
		}
		while (finder.toNextBoundary () > 0);
	}

	QString SpellcheckerExt::autoCorrectSuggestionForMisspelledWord (const QString& word)
	{
		if (!XmlSettingsManager::Instance ().property ("EnableAutocorrect").toBool ())
			return {};

		QStringList guesses;
		guessesForWord (word, {}, guesses);
		return guesses.value (0);
	}

	void SpellcheckerExt::guessesForWord (const QString& word, const QString&, QStringList& guesses)
	{
		for (const auto& sc : Spellcheckers_)
			if (!sc->IsCorrect (word))
				guesses += sc->GetPropositions (word);

		guesses.removeDuplicates ();
	}

	bool SpellcheckerExt::isGrammarCheckingEnabled ()
	{
		return false;
	}

	void SpellcheckerExt::toggleGrammarChecking ()
	{
	}

	void SpellcheckerExt::checkGrammarOfString (const QString&, QList<GrammarDetail>&, int*, int*)
	{
	}
}
}
