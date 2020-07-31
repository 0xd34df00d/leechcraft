/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QtPlugin>

class QString;

template<typename T>
class QList;

namespace LC
{
	struct Entity;
}

/** @brief Interface for plugins managing Advanced Notifications rules.
 *
 * This interface should be implemented by plugins that provide means to
 * store and manage the Advanced Notifications rules. Other plugins may
 * use this interface to query for different rules and their options.
 */
class IANRulesStorage
{
public:
	virtual ~IANRulesStorage () {}

	/** @brief Returns all rules matching the \em category.
	 *
	 * The category can be either one of the predefined categories (like
	 * LC::AN::CatIM) or empty, in which case all the rules are
	 * returned.
	 *
	 * @param[in] category The category of the rules to return, or empty
	 * for all rules.
	 * @return The list of all rules matching the given \em category.
	 */
	virtual QList<LC::Entity> GetAllRules (const QString& category) const = 0;

	/** @brief Requests opening the configuration of the given \em rule.
	 *
	 * @param[in] rule One of the rules returned from GetAllRules().
	 */
	virtual void RequestRuleConfiguration (const LC::Entity& rule) = 0;
protected:
	/** @brief Emitted when the rules change.
	 *
	 * This signal is emitted either when a new rule has been added, or an
	 * already existing rule has been deleted or modified.
	 *
	 * @note This function is expected to be a signal.
	 */
	virtual void rulesChanged () = 0;
};

Q_DECLARE_INTERFACE (IANRulesStorage, "org.LeechCraft.IANRulesStorage/1.0")
