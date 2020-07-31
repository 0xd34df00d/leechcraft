/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>

/** @brief An instance of a spell checker.
 *
 * This interface provides access to the spell checking capabilities of
 * plugins implementing ISpellCheckProvider and is obtained via a call
 * to ISpellCheckProvider::CreateSpellchecker().
 *
 * @sa ISpellCheckProvider
 */
class ISpellChecker
{
public:
	/** @brief Destroys the spell checker instance.
	 */
	virtual ~ISpellChecker () {}

	/** @brief Returns if the given \em word is correct.
	 *
	 * If the word is incorrect, it probably makes sense to get a list
	 * of possible corrections via GetPropositions().
	 *
	 * The spell checker uses the configuration of ISpellCheckProvider
	 * to figure out enabled languages, locales, spell checking options
	 * and so on.
	 *
	 * @param[in] word The word to check.
	 * @return Whether the word is correct.
	 *
	 * @sa GetPropositions()
	 * @sa LearnWord()
	 */
	virtual bool IsCorrect (const QString& word) const = 0;

	/** @brief Returns the list of propositions for the \em word.
	 *
	 * If the word is correct or no propositions are found, an empty
	 * list is returned.
	 *
	 * @param[in] word The word for which to return the list of
	 * propositions.
	 * @return The possibly empty list of propositions.
	 *
	 * @sa IsCorrect()
	 */
	virtual QStringList GetPropositions (const QString& word) const = 0;

	/** @brief Asks the spell checker to learn the given \em word.
	 *
	 * This function is allowed to do nothing if the spell checker does
	 * not support user dictionaries. In this case, IsCorrect() may still
	 * return <code>false</code> for the given \em word even after
	 * calling this function.
	 *
	 * @param[in] word The word to learn.
	 *
	 * @sa IsCorrect()
	 */
	virtual void LearnWord (const QString& word) = 0;
};

/** @brief A shared pointer to an ISpellChecker instance.
 */
using ISpellChecker_ptr = std::shared_ptr<ISpellChecker>;

/** @brief Interface for plugins providing spell checker capabilities.
 *
 * This interface itself does not expose anything for spell checking.
 * Instead, a spell checker instance should be requested via
 * CreateSpellchecker().
 *
 * Plugins implementing this interface have their own options for
 * languages, locales and so on. Thus, this interface offers no way to
 * tune these parameters.
 *
 * @sa ISpellChecker
 */
class ISpellCheckProvider
{
protected:
	virtual ~ISpellCheckProvider () {}
public:
	/** @brief Requests a new spellchecker.
	 *
	 * The ownership is passed to the caller.
	 *
	 * @return A ISpellChecker object.
	 */
	virtual ISpellChecker_ptr CreateSpellchecker () = 0;
};

Q_DECLARE_INTERFACE (ISpellChecker, "org.LeechCraft.ISpellChecker/1.0")
Q_DECLARE_INTERFACE (ISpellCheckProvider, "org.LeechCraft.ISpellCheckProvider/1.0")
