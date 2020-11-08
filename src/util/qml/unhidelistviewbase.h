/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QList>
#include <QQuickWidget>
#include <interfaces/core/icoreproxy.h>
#include "qmlconfig.h"

class QStandardItem;

namespace LC::Util
{
	class UnhideListModel;

	/** @brief Base class for a view of a list of items to be unclosed.
	 *
	 * This is a base class for widgets showing QML views with a list of
	 * items each of which can be unclosed, like a tab, a page, a button
	 * on a tab bar, and so on.
	 *
	 * The view uses UnhideListModel internally, please refer to its
	 * documentation regarding various data roles defined by the model.
	 *
	 * @sa UnhideListModel
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API UnhideListViewBase : public QQuickWidget
	{
		Q_OBJECT
	protected:
		UnhideListModel * const Model_;
	public:
		/** @brief Initializes the view and fills it with the items.
		 *
		 * The model is filled by invoking the given \em modelFiller
		 * function at a proper time, which should in turn append the
		 * items as needed to the model passed to it.
		 *
		 * The UnhideListModel is used as the model, so the passed
		 * \em modelFiller should set the appropriate data for the roles
		 * defined in UnhideListModel.
		 *
		 * @param[in] proxy The pointer to an ICoreProxy instance.
		 * @param[in] modelFiller A function filling the model with the
		 * items, or an empty function.
		 * @param[in] parent The parent widget of this view.
		 *
		 * @sa UnhideListModel
		 */
		UnhideListViewBase (const ICoreProxy_ptr& proxy,
				const std::function<void (UnhideListModel*)>& modelFiller,
				QWidget *parent = nullptr);

		/** @brief Sets the items of the view model to \em items.
		 *
		 * Replaces any items previously added via the model filler passed
		 * to the UnhideListViewBase constructor.
		 *
		 * The ownership of the \em items is transferred to the view.
		 *
		 * The UnhideListModel is used as the model, so the passed
		 * \em modelFiller should set the appropriate data for the roles
		 * defined in UnhideListModel.
		 *
		 * @param[in] items The items to be added to the view model.
		 *
		 * @sa UnhideListModel
		 */
		void SetItems (const QList<QStandardItem*>& items);
	signals:
		/** @brief Emitted when an item with the given \em itemId is
		 * activated.
		 *
		 * @param[out] itemId The ID of the activated item, equal to
		 * UnhideListModel::Roles::ItemClass.
		 *
		 * @sa UnhideListModel::Roles::ItemClass
		 */
		void itemUnhideRequested (const QString& itemId);
	};
}
