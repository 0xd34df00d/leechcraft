/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IMUCBOOKMARKEDITORWIDGET_H
#define PLUGINS_AZOTH_INTERFACES_IMUCBOOKMARKEDITORWIDGET_H
#include <QMetaType>
#include <QVariantMap>

namespace LC
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

Q_DECLARE_INTERFACE (LC::Azoth::IMUCBookmarkEditorWidget,
		"org.Deviant.LeechCraft.Azoth.IMUCBookmarkEditorWidget/1.0")

#endif
