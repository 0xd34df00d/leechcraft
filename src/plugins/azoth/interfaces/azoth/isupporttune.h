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

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTTUNE_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTTUNE_H
#include <QtGlobal>
#include <QMap>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for publishing user tunes.
	 * 
	 * This interface can be implemented by account objects (those that
	 * implement IAccount) to advertise the support for publishing
	 * currently playing tune information.
	 * 
	 * @sa IAccount
	 */
	class ISupportTune
	{
	public:
		virtual ~ISupportTune () {}
		
		/** @brief Publishes the currently listening music information.
		 * 
		 * The tuneData parameter is the map containing the following
		 * keys:
		 * - "artist" of type QString.
		 * - "title" of type QString.
		 * - "source" of type QString.
		 * - "length" of type QString.
		 */
		virtual void PublishTune (const QMap<QString, QVariant>& tuneData) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISupportTune,
		"org.Deviant.LeechCraft.Azoth.ISupportTune/1.0");

#endif
