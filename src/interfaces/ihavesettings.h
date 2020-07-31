/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_IHAVESETTINGS_H
#define INTERFACES_IHAVESETTINGS_H
#include <memory>
#include <QtPlugin>

namespace LC
{
	namespace Util
	{
		class XmlSettingsDialog;
		typedef std::shared_ptr<XmlSettingsDialog> XmlSettingsDialog_ptr;
	}
}

/** @brief Interface for plugins that have user-configurable settings.
 *
 * Plugins that have user-configurable settings should implement this
 * interface if they want to appear in a common settings configuration
 * dialog.
 */
class Q_DECL_EXPORT IHaveSettings
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
	virtual LC::Util::XmlSettingsDialog_ptr GetSettingsDialog () const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IHaveSettings () {}
};

Q_DECLARE_INTERFACE (IHaveSettings, "org.Deviant.LeechCraft.IHaveSettings/1.0")

#endif

