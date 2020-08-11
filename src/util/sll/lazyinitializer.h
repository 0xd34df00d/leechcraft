/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <optional>

namespace LC
{
namespace Util
{
	/** @brief Provides lazy initialization on first access to an object.
	 *
	 * If the lazy initializer is unevaluated, the initialization functor
	 * passed to the constructor will be called first time the object
	 * inside this initializer is accessed.
	 *
	 * @tparam Source The source object used for initialization of the
	 * object.
	 * @tparam Object The type of the object which should be initialized.
	 */
	template<typename Source, typename Object>
	class LazyInitializer
	{
		Source Source_;

		std::optional<Object> Object_;

		std::function<Object (Source)> Initializer_;
		std::function<void (Source&)> ClearSource_;
	public:
		/** @brief Constructs an unevaluated lazy initializer.
		 *
		 * @param[in] source The source object from which the \em Object
		 * should be initialized.
		 * @param[in] initializer The initialization function that returns
		 * an \em Object when called with \em Source.
		 * @param[in] clear The function for clearing the source after
		 * initialization to free up resources. Default function just
		 * assigns a default-constructed \em Source.
		 * @tparam Init The type of the \em initializer.
		 */
		template<typename Init>
		LazyInitializer (const Source& source,
				const Init& initializer,
				const std::function<void (Source&)>& clear = [] (Source& src) { src = Source {}; })
		: Source_ { source }
		, Initializer_ { initializer }
		, ClearSource_ { clear }
		{
		}

		/** @brief Constructs an evaluated initializer from the \em object.
		 *
		 * @param[in] object The object used to initialize the stored one.
		 */
		LazyInitializer (const Object& object)
		: Object_ { object }
		{
		}

		/** @brief Assigns an object to this lazy (making it evaluated)
		 * initializer and clears the source.
		 *
		 * @param[in] object The object to set this lazy initializer to.
		 */
		LazyInitializer& operator= (const Object& object)
		{
			Object_ = object;
			ClearSource_ (Source_);
			return *this;
		}

		/** @brief Conversion operator to \em Object, forcing object
		 * construction.
		 */
		operator Object ()
		{
			CheckInit ();
			return *Object_;
		}

		/** @brief Indirection operator, forcing object construction.
		 */
		Object& operator-> ()
		{
			CheckInit ();
			return *Object_;
		}

		void Force ()
		{
			CheckInit ();
		}
	private:
		void CheckInit ()
		{
			if (!Object_)
			{
				Object_ = Initializer_ (Source_);
				ClearSource_ (Source_);
			}
		}
	};
}
}
