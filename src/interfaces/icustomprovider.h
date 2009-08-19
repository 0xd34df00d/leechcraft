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

#ifndef INTERFACES_ICUSTOMPROVIDER_H
#define INTERFACES_ICUSTOMPROVIDER_H
#include <QString>

/** @brief Interface for plugins providing custom facilities.
 *
 * This interface should be used by plugins which provide custom
 * abilities not related to LeechCraft and not accounted by other
 * interfaces. All communication goes via signal/slot connections.
 */
class ICustomProvider
{
public:
	/** @brief Queries the plugin whether it implements a given feature.
	 *
	 * @param[in] feature Queried feature.
	 * @return Query result.
	 */
	virtual bool ImplementsFeature (const QString& feature) const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~ICustomProvider () {}
};

Q_DECLARE_INTERFACE (ICustomProvider, "org.Deviant.LeechCraft.ICustomProvider/1.0");

#endif

