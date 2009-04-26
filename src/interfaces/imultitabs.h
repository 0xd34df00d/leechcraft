#ifndef INTERFACES_IMULTITABS_H
#define INTERFACES_IMULTITABS_H
#include <QtPlugin>

/** @brief Interface for a single tab in a multitab plugin.
 *
 * Provides means to communicate with the single tab in a plugin.
 */
class IMultiTabsWidget
{
public:
	virtual ~IMultiTabsWidget () {}

	/** @brief Requests to remove the tab.
	 *
	 * This is usually called when a user has requested to close the tab
	 * via LeechCraft's UI. For example, clicking the "x" on the tab.
	 */
	virtual void Remove () = 0;
};

/** @brief Interface for plugins having (and opening/closing) multiple
 * tabs.
 *
 * When a plugin wants to add a new tab into LeechCraft, it emits the
 * addNewTab(const QString&, QWidget*) signal, where the first parameter
 * is the name of the new tab, and the second one is the pointer to the
 * widget with tab contents. Newly added widget would be reparented by
 * LeechCraft.
 * To remove a tab, it emits removeTab(QWidget*), where the parameter is
 * the pointer to a previously added tab's widget.
 * To change tab's name, plugin emits changeTabName(QWidget*, const
 * QString&), where the first parameter is the pointer to previously
 * inserted tab and the second one is the new name.
 * To change tab's icon, plugin emits changeTabIcon(QWidget*, const
 * QIcon&), where the first parameter is the pointer to previously
 * inserted tab and the seocnd one is the new icon.
 * To bring the tab to front, plugin emits raiseTab(QWidget*) signal,
 * where the first parameter is the pointer to previously inserted tab.
 *
 * @sa IEmbedTab
 * @sa IWindow
 */
class IMultiTabs
{
public:
	/** @brief Virtual destructor.
	 */
	virtual ~IMultiTabs () {}
};

Q_DECLARE_INTERFACE (IMultiTabsWidget, "org.Deviant.LeechCraft.IMultiTabsWidget/1.0");
Q_DECLARE_INTERFACE (IMultiTabs, "org.Deviant.LeechCraft.IMultiTabs/1.0");

#endif

