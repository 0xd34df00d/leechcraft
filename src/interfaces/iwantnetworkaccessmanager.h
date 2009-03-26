#ifndef IWANTNETWORKACCESSMANAGER_H
#define IWANTNETWORKACCESSMANAGER_H
#include <QtPlugin>

class QNetworkAccessManager;

/** @brief Interface for plugins which want to share network access
 * manager.
 *
 * The network access manager shares cache, cookies and other stuff
 * between plugins, so it's often handy to have one network access
 * manager for all plugins.
 */
class IWantNetworkAccessManager
{
public:
	/** @brief Sets the network access manager.
	 *
	 * The passed network manager object should be still owned by the
	 * LeechCraft, plugin shouldn't take the ownership.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @param nam[in] The QNetworkAccessManager object that's shared
	 * among the plugins.
	 */
	virtual void SetNetworkAccessManager (QNetworkAccessManager *nam) = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IWantNetworkAccessManager () {}
};

Q_DECLARE_INTERFACE (IWantNetworkAccessManager, "org.Deviant.LeechCraft.IWantNetworkAccessManager/1.0");

#endif

