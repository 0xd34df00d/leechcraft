/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QFuture>
#include <util/sll/newtype.h>

namespace LC
{
namespace Util
{
	using QtException_t = QException;

	using QtException_ptr = std::shared_ptr<QtException_t>;

	/** @brief A concurrent exception that plays nicely with Qt.
	 *
	 * This class can be used to make some third-party exception type
	 * compatible with the QtConcurrent framework, which requires all
	 * exceptions to be derived from a Qt's base exception class.
	 *
	 * This class wraps differences between Qt4's QtConcurrent::Exception
	 * and Qt5's QException and provides an uniform interface.
	 *
	 * @tparam T The base type of the exception.
	 */
	template<typename T>
	class ConcurrentException : public QtException_t
							  , public T
	{
	public:
		/** @brief Default-constructs the exception object.
		 */
		ConcurrentException () = default;

		/** @brief Constructs the exception object with the given
		 * \em args.
		 *
		 * @param[in] args The argments to pass to the constructor of the
		 * base exception \em T.
		 * @tparam Args The types of the arguments of the constructor of
		 * \em T.
		 */
		template<typename... Args>
		requires requires (Args&&... args) { T { std::forward<Args> (args)... }; }
		ConcurrentException (Args&&... args)
		: T { std::forward<Args> (args)... }
		{
		}

		/** @brief Rethrows an exception of exactly this type and state.
		 *
		 * @throws ConcurrentException<T>
		 */
		void raise () const override
		{
			throw ConcurrentException<T> { *this };
		}

		/** @brief Constructs a copy of this object.
		 *
		 * @return The copy of this object.
		 */
		ConcurrentException<T>* clone () const override
		{
			return new ConcurrentException<T> { *this };
		}

		/** @brief Overrides base pure virtual.
		 *
		 * Calls <code>T::what()</code>.
		 */
		const char* what () const noexcept override
		{
			return T::what ();
		}
	};

	using ConcurrentStdException = Util::ConcurrentException<Util::NewType<std::exception, struct StdException>>;
}
}

Q_DECLARE_METATYPE (LC::Util::QtException_ptr)
