/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ILASTACTIVITYPROVIDER_H
#define PLUGINS_AZOTH_INTERFACES_ILASTACTIVITYPROVIDER_H
#include <QMetaType>

namespace LC
{
namespace Azoth
{
	/** @brief Interface for plugins providing last activity info.
	 *
	 * This interface should be implemented by plugins (yes, plugin
	 * instance objects) that may provide information about
	 * inactivity timeout.
	 */
	class ILastActivityProvider
	{
	public:
		virtual ~ILastActivityProvider () {}

		/** @brief Number of seconds of inactivity.
		 *
		 * This method returns the number of seconds the user has been
		 * inactive.
		 *
		 * @return Number of seconds of inactivity.
		 */
		virtual int GetInactiveSeconds () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ILastActivityProvider,
		"org.Deviant.LeechCraft.Azoth.ILastActivityProvider/1.0")

#endif
