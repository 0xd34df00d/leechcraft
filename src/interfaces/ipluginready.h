#ifndef IPLUGINREADY_H
#define IPLUGINREADY_H
#include <QtPlugin>

class IPluginReady
{
public:
	/** @brief Returns the expected class of the plugins for this
	 * plugin.
	 *
	 * Returns the expected second level plugins' class expected by this
	 * first-level plugin.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return The expected plugin class entity.
	 */
	virtual QByteArray GetExpectedPluginClass () const = 0;

	/** @brief Adds second-level plugin to this one.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @param[in] plugin The pointer to the plugin instance.
	 */
	virtual void AddPlugin (QObject *plugin) = 0;
};

Q_DECLARE_INTERFACE (IPluginReady, "org.Deviant.LeechCraft.IPluginReady/1.0");

#endif

