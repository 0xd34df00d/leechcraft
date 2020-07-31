/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>

namespace LC
{
namespace Azoth
{
	struct ActivityInfo;

	/** @brief Interface for accounts supporting user activity.
	 *
	 * This interface can be implemented by account objects to advertise
	 * the support for publishing current user activity.
	 *
	 * The activities concept in Azoth is based on the XMPP XEP-0108:
	 * User Activities (http://xmpp.org/extensions/xep-0108.html).
	 *
	 * @sa IAccount
	 * @sa ActivityInfo
	 */
	class ISupportActivity
	{
	public:
		virtual ~ISupportActivity () {}

		/** @brief Publishes the current user activity.
		 *
		 * @param[in] info The activity description.
		 */
		virtual void SetActivity (const ActivityInfo& info) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportActivity,
		"org.Deviant.LeechCraft.Azoth.ISupportActivity/1.0")
