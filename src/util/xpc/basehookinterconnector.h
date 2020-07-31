/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QList>
#include "xpcconfig.h"

namespace LC
{
namespace Util
{
	/** @brief Base class for signal-slot relays in plugin APIs.
	 *
	 * This class is typically used by plugins that support other plugins
	 * to relay signals from the internals of the root plugin to the
	 * subplugins.
	 *
	 * The interconnector automatically connects its signals with the
	 * matching slots (that is, with the same name and parameters) of
	 * subplugins added to it via AddPlugin(), and connects signals of
	 * the objects added via RegisterHookable() to its own matching
	 * signals.
	 *
	 * The usage is as follows:
	 * -# One subclasses from this class and lists all the signals that
	 *    should ever be relayed to plugins in the signals section of the
	 *    subclass. First parameter of the signals should always be
	 *    LC::IHookProxy_ptr.
	 * -# One adds the instance objects of the subplugins to the
	 *    interconnector via the AddPlugin() method.
	 * -# One registers the objects emitting signals that need to be
	 *    relayed to plugins via the RegisterHookable() method.
	 * -# Everything works.
	 *
	 * Please note that second and third steps can be done in arbitrary
	 * order and even be interleaved.
	 */
	class UTIL_XPC_API BaseHookInterconnector : public QObject
	{
		Q_OBJECT
	protected:
		QList<QObject*> Plugins_;
	public:
		/** @brief Creates the interconnector with the given parent.
		 *
		 * @param[in] parent The parent object of this interconnector.
		 */
		BaseHookInterconnector (QObject *parent = 0);

		/** @brief Virtual destructor.
		 */
		virtual ~BaseHookInterconnector ();

		/** @brief Adds a subplugin to this interconnector.
		 *
		 * This function is used to add a subplugin whose slots should
		 * be connected to the signals of this plugin.
		 *
		 * Every signal of this object with the name and parameters list
		 * matching a slot of \em plugin will be automatically connected
		 * to it.
		 *
		 * @param[in] plugin The subplugin to add to this interconnector.
		 *
		 * @sa RegisterHookable()
		 */
		virtual void AddPlugin (QObject *plugin);

		/** @brief Adds a hookable object from the root plugin.
		 *
		 * This function is used to add an object whose signals need to
		 * be exposed to subplugins.
		 *
		 * Every signal of \em hookable with the name and parameters list
		 * matching a signal of this object will be automatically
		 * connected to it.
		 *
		 * @param[in] hookable The hookable object from the root plugin
		 * to add.
		 *
		 * @sa AddPlugin()
		 */
		void RegisterHookable (QObject *hookable);
	};
}
}
