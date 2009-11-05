/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef INTERFACES_ISTARTUPWIZARD_H
#define INTERFACES_ISTARTUPWIZARD_H
#include <QList>

class QWizardPage;

/** @brief Interface for adding page to the common startup wizard.
 *
 * If your plugin wishes to show a startup wizard in a uniform way, it
 * should implement this interface and return the pages it wishes to
 * insert into the wizard in GetWizardPages(). Of course, it can return
 * different pages each time.
 */
class IStartupWizard
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

Q_DECLARE_INTERFACE (IStartupWizard, "org.Deviant.LeechCraft.IStartupWizard/1.0");

#endif

