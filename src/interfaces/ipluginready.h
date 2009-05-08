#ifndef INTERFACES_IPLUGINREADY_H
#define INTERFACES_IPLUGINREADY_H
#include <QtPlugin>

/** @brief Base class for plugins accepting second-level plugins.
 *
 * A plugin for LeechCraft could be actually a plugin for another
 * plugin. Then, to simplify the process, if a plugin could handle such
 * second-level plugins (if it's a host for them), it's better to
 * implement this interface. LeechCraft would the automatically manage
 * the dependencies, perform correct initialization order and feed the
 * matching first-level plugins with second-level ones.
 *
 * Plugins of different levels are matched with each other by their
 * class, which is returned by IPlugin2::GetPluginClass() and by
 * IPluginReady::GetExpectedPluginClass().
 */
class IPluginReady
{
public:
	/** @brief Returns the expected class of the plugins for this
	 * plugin.
	 *
	 * Returns the expected second level plugins' class expected by this
	 * first-level plugin.
	 *
	 * @note This function should be able to work before IInfo::Init() is
	 * called.
	 *
	 * @return The expected plugin class entity.
	 */
	virtual QByteArray GetExpectedPluginClass () const = 0;

	/** @brief Adds second-level plugin to this one.
	 *
	 * @note This function should be able to work before IInfo::Init() is
	 * called.
	 *
	 * @param[in] plugin The pointer to the plugin instance.
	 */
	virtual void AddPlugin (QObject *plugin) = 0;
};

Q_DECLARE_INTERFACE (IPluginReady, "org.Deviant.LeechCraft.IPluginReady/1.0");

#endif

