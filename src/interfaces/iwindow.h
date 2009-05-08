#ifndef INTERFACES_IWINDOW_H
#define INTERFACES_IWINDOW_H
#include <QWidget>
#include <QtPlugin>

/** @brief Interface for plugins with their own main windows.
 *
 * If a plugin creates a main window and wants to show it upon some user
 * actions (like double-clicking plugin's name in plugins list), it
 * should implement this interface.
 */
class IWindow
{
public:
	/** @brief Sets the parent widget of the window.
	 *
	 * This function is called by the LeechCraft to inform the plugin
	 * about its parent widget.
	 *
	 * @param[in] parent Pointer to parent widget.
	 */
    virtual void SetParent (QWidget *parent) = 0;

	/** @brief Shows the plugin's main window.
	 *
	 * This function is called by LeechCraft when the user has done an
	 * action which means that the plugin should show or hide it's
	 * window (depending of the current state).
	 */
    virtual void ShowWindow () = 0;

	/** @brief Virtual destructor.
	 */
    virtual ~IWindow () {}
};

Q_DECLARE_INTERFACE (IWindow, "org.Deviant.LeechCraft.IWindow/1.0");

#endif

