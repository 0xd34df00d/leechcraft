#ifndef INTERFACES_IPLUGIN2_H
#define INTERFACES_IPLUGIN2_H
#include <QtPlugin>
#include <QByteArray>

/** @brief Base class for second-level plugins.
 *
 * A plugin for LeechCraft could be actually a plugin for another
 * plugin. Then, to simplify the process, it's better to implement this
 * interface. LeechCraft would the automatically manage the
 * dependencies, perform correct initialization order and feed the
 * matching first-level plugins with second-level ones.
 *
 * Plugins of different levels are matched with each other by their
 * class, which is returned by IPlugin2::GetPluginClass() and by
 * IPluginReady::GetExpectedPluginClass().
 */
class IPlugin2
{
public:
	/** @brief Returns the plugin class of this second-level plugin.
	 *
	 * @note This function should be able to work before IInfo::Init()
	 * is called.
	 *
	 * @return The plugin class.
	 */
	virtual QByteArray GetPluginClass () const = 0;

	virtual ~IPlugin2 () {}
};

Q_DECLARE_INTERFACE (IPlugin2, "org.Deviant.LeechCraft.IPlugin2/1.0");

#endif

