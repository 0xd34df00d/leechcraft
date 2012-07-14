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

#ifndef PLUGINS_AZOTH_INTERFACES_IMUCBOOKMARKEDITORWIDGET_H
#define PLUGINS_AZOTH_INTERFACES_IMUCBOOKMARKEDITORWIDGET_H
#include <QMetaType>
#include <QVariantMap>

namespace LeechCraft
{
namespace Azoth
{
	class IMUCBookmarkEditorWidget
	{
	public:
		virtual ~IMUCBookmarkEditorWidget () {}
		
		/** @brief Returns the map with current join parameters.
		 * 
		 * This function is completely analogous to the
		 * IMUCJoinWidget::GetIdentifyingData() function. Refer to its
		 * documentation for more information.
		 * 
		 * @return Join parameters map.
		 * 
		 * @sa IMUCJoinWidget::GetIdentifyingData()
		 */
		virtual QVariantMap GetIdentifyingData () const = 0;
		
		/** @brief Sets the previously saved join parameters.
		 * 
		 * This function is completely analogous to the
		 * IMUCJoinWidget::SetIdentifyingData() function. Refer to its
		 * documentation for more information.
		 * 
		 * @param[in] data Join parameters map.
		 * 
		 * @sa IMUCJoinWidget::SetIdentifyingData()
		 */
		virtual void SetIdentifyingData (const QVariantMap& data) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMUCBookmarkEditorWidget,
		"org.Deviant.LeechCraft.Azoth.IMUCBookmarkEditorWidget/1.0");

#endif
