/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IPROTOCOLPLUGIN_H
#define PLUGINS_AZOTH_INTERFACES_IPROTOCOLPLUGIN_H
#include <QList>

class QObject;

namespace LC
{
namespace Azoth
{
	/** This is the base interface for plugins providing messaging
	 * protocols. Since these plugins are plugins for plugins, they
	 * should also implement IPlugin2 and return the
	 * "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin"
	 * string, among others, from their IPlugin2::GetPluginClasses()
	 * method.
	 */
	class IProtocolPlugin
	{
	public:
		virtual ~IProtocolPlugin () {}

		/** @brief Returns the protocol plugin object as a QObject.
		 *
		 * @return The protocol plugin as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns the protocols list provided by this plugin.
		 *
		 * Each object in this list must implement the IProtocol
		 * interface.
		 *
		 * @return The list of this plugin's protocols.
		 *
		 * @sa IProtocol, gotNewProtocols()
		 */
		virtual QList<QObject*> GetProtocols () const = 0;
	protected:
		/** @brief Notifies Azoth that new protocols are available.
		 *
		 * Each object in the protocols list must implement the
		 * IProtocol interface.
		 *
		 * After this signal the protocols should be contained in the
		 * list returned by GetProtocols().
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] protocols The new protocols made available in
		 * this protocol plugin.
		 */
		virtual void gotNewProtocols (const QList<QObject*>& protocols) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IProtocolPlugin,
		"org.Deviant.LeechCraft.Azoth.IProtocolPlugin/1.0")

#endif

