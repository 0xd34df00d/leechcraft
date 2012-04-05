/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTACTIVITY_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTACTIVITY_H
#include <QtGlobal>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for accounts supporting user activity.
	 * 
	 * This interface can be implemented by account objects to advertise
	 * the support for publishing current user activity.
	 * 
	 * The activities concept in Azoth is based on the XMPP XEP-0108:
	 * User Activities (http://xmpp.org/extensions/xep-0108.html).
	 * 
	 * @sa IAccount
	 */
	class ISupportActivity
	{
	public:
		virtual ~ISupportActivity () {}
		
		/** @brief Publishes the current user activity.
		 * 
		 * The activity information is divided into three pieces:
		 * general activity (required), specific activity (optional) and
		 * an optional text.
		 * 
		 * The possible values of the general and specific fields are
		 * listed in http://xmpp.org/extensions/xep-0108.html.
		 * 
		 * @param[in] general The general activity.
		 * @param[in] specific The specific activity (optional).
		 * @param[in] text The additional text message (optional).
		 */
		virtual void SetActivity (const QString& general,
				const QString& specific, const QString& text) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISupportActivity,
		"org.Deviant.LeechCraft.Azoth.ISupportActivity/1.0");

#endif
