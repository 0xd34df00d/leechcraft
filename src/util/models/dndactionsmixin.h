/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <utility>
#include <QAbstractItemModel>

namespace LC
{
namespace Util
{
	/** @brief Abstracts away differences between Qt4 and Qt5 in model
	 * DnD support.
	 *
	 * Derive from this class (and parametrize the template by desired
	 * base class like QStandardItemModel or QAbstractItemModel) to
	 * obtain Qt4-style getter and setter functions for supported drag
	 * and drop actions (supportedDragActions(),
	 * setSupportedDragActions(), supportedDropActions(),
	 * setSupportedDropActions()).
	 *
	 * @tparam Model The source model type that you wish to derive from.
	 *
	 * @sa supportedDragActions()
	 * @sa setSupportedDragActions()
	 * @sa supportedDropActions()
	 * @sa setSupportedDropActions()
	 *
	 * @ingroup ModelUtil
	 */
	template<typename Model>
	class DndActionsMixin : public Model
	{
		Qt::DropActions Drags_;
		Qt::DropActions Drops_;
	public:
		/** @brief Constructs the model passing the arguments to the base
		 * constructor.
		 *
		 * @param[in] args The list of parameters to pass to the
		 * constructor of the base \em Model.
		 * @tparam Args The variadic template parameter pack of arguments
		 * for the base \em Model constructor.
		 */
		template<typename... Args>
		DndActionsMixin (Args&&... args)
		: Model { std::forward<Args> (args)... }
		, Drags_ { Model::supportedDragActions () }
		, Drops_ { Model::supportedDropActions () }
		{
		}

		Qt::DropActions supportedDragActions () const override
		{
			return Drags_;
		}

		void setSupportedDragActions (Qt::DropActions acts)
		{
			Drags_ = acts;
		}

		Qt::DropActions supportedDropActions () const override
		{
			return Drops_;
		}

		void setSupportedDropActions (Qt::DropActions acts)
		{
			Drops_ = acts;
		}
	};
}
}
