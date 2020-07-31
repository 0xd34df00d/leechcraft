/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>

class QWizardPage;

/** @brief Interface for adding page to the common startup wizard.
 *
 * If your plugin wishes to show a startup wizard in a uniform way, it
 * should implement this interface and return the pages it wishes to
 * insert into the wizard in GetWizardPages(). Of course, it can return
 * different pages each time.
 */
class Q_DECL_EXPORT IStartupWizard
{
public:
	virtual ~IStartupWizard () {}

	/** Returns the list of pages to insert into the wizard.
	 *
	 * Pages in the returned list will be inserted into the wizard in
	 * the same order as they appear in the list. Order of different
	 * sets of pages returned by different plugins is arbitrary.
	 *
	 * Plugin can return different pages each time this function is
	 * called.
	 *
	 * @return The list of pages to insert into the wizard.
	 */
	virtual QList<QWizardPage*> GetWizardPages () const = 0;
};

Q_DECLARE_INTERFACE (IStartupWizard, "org.Deviant.LeechCraft.IStartupWizard/1.0")
