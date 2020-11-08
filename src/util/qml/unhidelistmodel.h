/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <util/models/rolenamesmixin.h>
#include "qmlconfig.h"

namespace LC::Util
{
	/** @brief A model to be used with UnhideListViewBase.
	 *
	 * This model just defines some roles used in various "unhide" lists
	 * and exposes them to QML.
	 *
	 * @sa UnhideListViewBase
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API UnhideListModel : public RoleNamesMixin<QStandardItemModel>
	{
	public:
		/** @brief Various unhide roles to be set by the rows of this
		 * model.
		 */
		enum Roles
		{
			/** @brief The unique ID of the item represented by this row.
			 *
			 * Exposed under the name \em itemClass to QML.
			 */
			ItemClass = Qt::UserRole + 1,

			/** @brief The name of the item represented by this row
			 *
			 * Exposed under the name \em itemName to QML.
			 */
			ItemName,

			/** @brief The description of the item represented by this row.
			 *
			 * Exposed under the name \em itemDescr to QML.
			 */
			ItemDescription,

			/** @brief The URL of the icon of the item represented by this
			 * row.
			 *
			 * Exposed under the name \em itemIcon to QML.
			 */
			ItemIcon
		};

		/** @brief Constructs the model with the given \em parent.
		 *
		 * @param[in] parent The parent object of this model.
		 */
		explicit UnhideListModel (QObject *parent);
	};
}
