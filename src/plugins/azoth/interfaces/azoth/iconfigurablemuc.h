/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ICONFIGURABLEMUC_H
#define PLUGINS_AZOTH_INTERFACES_ICONFIGURABLEMUC_H
#include <QMetaType>

class QWidget;

namespace LC
{
namespace Azoth
{
	/** @brief This interface is for widgets used to configure the room.
	 * 
	 * The widgets that are returned from the
	 * IConfigurableMUC::GetConfigurationWidget() method must implement
	 * this interface.
	 * 
	 * They are shown to the user, and since it's better to pre-fill the
	 * widget with current room configuration, but fetching the
	 * configuration may require communicating to the server, the widget
	 * would be in disabled state until the dataReady() signal is
	 * emitted.
	 * 
	 * When the user chooses to accept the widget, then accept() method
	 * or IConfigurableMUC::AcceptConfiguration() method would be called
	 * depending on the context, but never both at once. It's suggested
	 * that one should just be implemented by calling the other.
	 */
	class IMUCConfigWidget
	{
	public:
		virtual ~IMUCConfigWidget () {}
		
		/** @brief This function is called to accept the configuration.
		 * 
		 * If the user chooses to accept the configuration, then this
		 * function is called.
		 * 
		 * @note This function is expected to be a slot.
		 */
		virtual void accept () = 0;
		
		/** @brief This signal notifies about the widget's readiness.
		 * 
		 * When the current room configuration data arrives and the
		 * widget is filled with it, this signal should be emitted to
		 * notify Azoth that it's now safe to allow the user to interact
		 * with the widget.
		 * 
		 * @note This function is expected to be a signal.
		 */
		virtual void dataReady () = 0;
	};

	/** @brief This interfaces is used for MUCs that can be configured.
	 * 
	 * If a multiuser chatroom can be configured by the user, than it
	 * can implement this interface for the Azoth to promote this
	 * ability to the user. Obviously, the chatroom should also
	 * implement ICLEntry and IMUCEntry interfaces.
	 */
	class IConfigurableMUC
	{
	public:
		virtual ~IConfigurableMUC () {}
		
		/** @brief Returns the widget used for configuration.
		 * 
		 * The returned widget is shown to the user to be filled by him,
		 * and if the user accepts the new configuration, the
		 * AcceptConfiguration() of this method would be called (or
		 * IMUCConfigWidget::accept(), depending on context).
		 * 
		 * Obviously, it's better to pre-fill the widget with the values
		 * corresponding to the current configuration of the MUC room.
		 * Since the filling may require communicating to the server,
		 * the widget is assumed to be in the invalid state until
		 * its dataReady() signal is emitted. Until that signal is
		 * emitted, user won't be allowed to interact with the widget.
		 * 
		 * This function should create a new widget each time it is
		 * called, and the ownership is transferred to the caller.
		 * 
		 * The returned widget must implement IMUCConfigWidget.
		 * 
		 * @return The widget used to configure the room, implementing
		 * IMUCConfigWidget.
		 * 
		 * @sa AcceptConfiguration()
		 */
		virtual QWidget* GetConfigurationWidget () = 0;

		/** @brief Accepts the configuration.
		 * 
		 * If the user has accepted the configuration, this method is
		 * called with the widget being the one returned previously
		 * from the GetConfigurationWidget(). In this method, for
		 * example, it could be qobject_cast'ed to the exact widget
		 * type, and the values entered by the user used to update the
		 * configuration of the room.
		 * 
		 * @param[in] widget The widget previously returned from
		 * GetConfigurationWidget() and filled by the user.
		 * 
		 * @sa GetConfigurationWidget()
		 */
		virtual void AcceptConfiguration (QWidget *widget) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IMUCConfigWidget,
		"org.Deviant.LeechCraft.Azoth.IMUCConfigWidget/1.0")
Q_DECLARE_INTERFACE (LC::Azoth::IConfigurableMUC,
		"org.Deviant.LeechCraft.Azoth.IConfigurableMUC/1.0")

#endif
