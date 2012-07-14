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

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTMOOD_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTMOOD_H
#include <QtGlobal>

class QString;

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for accounts supporting user mood.
	 * 
	 * This interface can be implemented by account objects to advertise
	 * the support for publishing current user mood.
	 * 
	 * The mood concept in Azoth is based on the XMPP XEP-0107: User
	 * Mood (http://xmpp.org/extensions/xep-0107.html).
	 * 
	 * @sa IAccount
	 */
	class ISupportMood
	{
	public:
		virtual ~ISupportMood () {}
		
		/** @brief Publishes the current user mood.
		 * 
		 * The mood information is divided into two pieces:
		 * mood name (required) and an optional text.
		 * 
		 * The possible values of the mood name are
		 * listed in http://xmpp.org/extensions/xep-0107.html.
		 * 
		 * @param[in] mood The mood name.
		 * @param[in] text The additional text message (optional).
		 */
		virtual void SetMood (const QString& mood, const QString& text) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISupportMood,
		"org.Deviant.LeechCraft.Azoth.ISupportMood/1.0");

#endif
