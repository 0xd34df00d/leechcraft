#ifndef IHAVESETTINGS_H
#define IHAVESETTINGS_H
#include <QtPlugin>

namespace LeechCraft
{
	namespace Util
	{
		class XmlSettingsDialog;
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
	virtual LeechCraft::Util::XmlSettingsDialog* GetSettingsDialog () const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IHaveSettings () {}
};

Q_DECLARE_INTERFACE (IHaveSettings, "org.Deviant.LeechCraft.IHaveSettings/1.0");

#endif

