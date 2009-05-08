#ifndef INTERFACES_ITOOLBAREMBEDDER_H
#define INTERFACES_ITOOLBAREMBEDDER_H
#include <QList>
#include <QAction>

/** @brief Interface to embed actions into the main toolbar.
 *
 * Plugins that want to embed some actions into the main LeechCraft
 * window's toolbar. Actions are appended to the end of the toolbar, so
 * the order of them depends upon plugin load order.
 */
class IToolBarEmbedder
{
public:
	/** @brief Returns the list of actions.
	 *
	 * Returns the list of pointers to QActions that should be embedded
	 * into the toolbar.
	 *
	 * @return The list of actions.
	 */
	virtual QList<QAction*> GetActions () const = 0; 

	virtual ~IToolBarEmbedder () {}
};

Q_DECLARE_INTERFACE (IToolBarEmbedder, "org.Deviant.LeechCraft.IToolBarEmbedder/1.0");

#endif

