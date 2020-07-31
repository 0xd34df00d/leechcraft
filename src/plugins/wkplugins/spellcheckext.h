/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebKitPlatformPlugin>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/ispellcheckprovider.h>

namespace LC
{
namespace WKPlugins
{
	class SpellcheckerExt : public QWebSpellChecker
	{
		const ICoreProxy_ptr Proxy_;

		static QList<ISpellChecker_ptr> Spellcheckers_;

		bool ContinousEnabled_ = true;

		QStringList Ignored_;
	public:
		SpellcheckerExt (const ICoreProxy_ptr&);

		bool isContinousSpellCheckingEnabled () const override;
		void toggleContinousSpellChecking () override;

		void learnWord (const QString& word) override;
		void ignoreWordInSpellDocument (const QString& word) override;
		void checkSpellingOfString (const QString& word, int *misspellingLocation, int *misspellingLength) override;
		QString autoCorrectSuggestionForMisspelledWord (const QString& word) override;
		void guessesForWord (const QString& word, const QString& context, QStringList& guesses) override;

		bool isGrammarCheckingEnabled () override;
		void toggleGrammarChecking () override;
		void checkGrammarOfString (const QString&, QList<GrammarDetail>&,
				int *badGrammarLocation, int *badGrammarLength) override;
	};
}
}
