/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef INTERFACES_IHAVESETTINGS_H
#define INTERFACES_IHAVESETTINGS_H
#include <boost/shared_ptr.hpp>
#include <QtPlugin>

namespace LeechCraft
{
	namespace Util
	{
		class XmlSettingsDialog;
		typedef boost::shared_ptr<XmlSettingsDialog> XmlSettingsDialog_ptr;
	};
};

/** @brief Interface for plugins that have user-configurable settings.
 *
 * Plugins that have user-configurable settings should implement this
 * interface if they want to appear in a common settings configuration
 * dialog.
 */
class IHaveSettings
{
public:
	/** @brief Gets the settings dialog manager object from the plugin.
	 *
	 * The returned XmlSettingsDialog would be integrated into common
	 * settings dialog where user can configure all the plugins that
	 * provide this interface.
	 *
	 * @return The XmlSettingsDialog object that manages the settings
	 * dialog of the plugin.
	 */
	virtual LeechCraft::Util::XmlSettingsDialog_ptr GetSettingsDialog () const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IHaveSettings () {}
};

Q_DECLARE_INTERFACE (IHaveSettings, "org.Deviant.LeechCraft.IHaveSettings/1.0");

#endif

