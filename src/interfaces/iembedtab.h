#ifndef INTERFACES_IEMBEDTAB_H
#define INTERFACES_IEMBEDTAB_H
#include <QtPlugin>

class QWidget;
class QToolBar;

/** @brief Interface for plugins embedding a tab into main LeechCraft's
 * window.
 *
 * Implementing this interface means that plugin wants to embed a tab
 * into LeechCraft's main window. IInfo::GetName() would be used as a
 * name for the tab. If your plugin could open/close multiple tabs, have
 * a look at IMultiTabs.
 *
 * Plugin is expected to implement following signals:
 * - addNewTab(const QString&,QWidget*) which adds a new tab with the
 *   given name and widget contents.
 * - removeTab(QWidget*) which removes tab with given contents.
 * - changeTabName(QWidget*,const QString&) which changes tab name of
 *   the tab with the given widget.
 * - changeTabIcon(QWidget*,const QIcon&) which changes the icon of the
 *   tab with the given widget.
 * - statusBarChanged(QWidget*,const QString&) notifies that the status
 *   bar message of the given widget is changed. Note that the message
 *   would be updated only if the given widget is visible.
 *
 * @sa IMultiTabs
 * @sa IWindow
 */
class IEmbedTab
{
public:
	/** @brief Returns the widget with tab contents.
	 *
	 * @return Widget with tab contents.
	 */
	virtual QWidget* GetTabContents () = 0;

	/** @brief Requests plugin's toolbar.
	 *
	 * The returned toolbar would be shown on top of the LeechCraft's
	 * main window. If there is no toolbar, 0 should be returned.
	 *
	 * @return The toolbar of this plugin.
	 */
	virtual QToolBar* GetToolBar () const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IEmbedTab () {}
};

Q_DECLARE_INTERFACE (IEmbedTab, "org.Deviant.LeechCraft.IEmbedTab/1.0");

#endif

