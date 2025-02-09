/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <utility>
#include <QHash>

namespace LC::Util
{
	/** @brief Provides Qt4-style role names setters.
	 *
	 * Qt4 uses QAbstractItemModel::setRoleNames() to set role names for
	 * a (non-virtual) QAbstractItemModel::roleNames() method. Newer Qt, on
	 * the other hand, just suggests overriding the (virtual) roleNames()
	 * method. This class provides an uniform interface. This class
	 * provides an uniform interface for role names setting.
	 *
	 * @tparam Model The original model type to derive from.
	 *
	 * @ingroup ModelUtil
	 */
	template<typename Model>
	class RoleNamesMixin : public Model
	{
		QHash<int, QByteArray> RoleNames_;
	protected:
		using Model::Model;

		/** @brief Sets the role names to \em roleNames.
		 *
		 * @param[in] roleNames The mapping from role value to its name.
		 *
		 * @sa roleNames()
		 */
		void setRoleNames (const QHash<int, QByteArray>& roleNames)
		{
			RoleNames_ = roleNames;
		}

		/** @brief Returns the role names.
		 *
		 * Returns the mapping from the role value to its name that was
		 * previously set via setRoleNames().
		 *
		 * @return The mapping from role value to its name.
		 *
		 * @sa setRoleNames()
		 */
		QHash<int, QByteArray> roleNames () const override
		{
			return RoleNames_;
		}
	};
}
