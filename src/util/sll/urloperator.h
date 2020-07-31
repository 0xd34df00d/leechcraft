/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QUrl>
#include <QUrlQuery>
#include "sllconfig.h"

namespace LC
{
namespace Util
{
	/** @brief Manipulates query part of an QUrl object.
	 *
	 * This class abstracts away differences between Qt4 and Qt5 QUrl and
	 * QUrlQuery handling, and it should be used in all new code instead
	 * of direct calls to Qt API.
	 *
	 * This class is used as follows:
	 * -# An object of this class is constructed on a (named) QUrl object.
	 * -# New URL query parameters are added by calling this object with a
	 *    pair of matching key and value.
	 * -# Existing URL query parameters are removed via the -= operator.
	 * -# The URL is updated on UrlOperator object destruction.
	 *
	 * Intended usage:
	 * \code{.cpp}
		QUrl someUrl { ... };
		UrlOperator { someUrl }
				("key1", "value1")
				("key2", "value2");
	   \endcode
	 *
	 * Here, an unnamed UrlOperator object is created that is valid only
	 * inside the corresponding expression, thus the changes to
	 * <code>someUrl</code> are visible immediately after executing that line.
	 *
	 * @note The changes are \em guaranteed to be applied on UrlOperator
	 * object destruction. Nevertheless, they may still be applied
	 * earlier on during calls to operator()() and operator-=().
	 */
	class UTIL_SLL_API UrlOperator
	{
		QUrl& Url_;

		QUrlQuery Query_;
	public:
		/** @brief Constructs the object modifying the query of \em url.
		 *
		 * @param[in] url The URL to modify.
		 */
		UrlOperator (QUrl& url);

		/** @brief Flushes any pending changes to the QUrl query and
		 * destroys the UrlOperator.
		 *
		 * @sa Flush()
		 * @sa operator()()
		 */
		~UrlOperator ();

		/** @brief Flushes any pending changes to the QUrl query.
		 */
		void Flush ();

		/** @brief Adds a new \em key = \em value parameters pair.
		 *
		 * If the URL already contains this \em key, a new value is added
		 * in addition to the already existing one.
		 *
		 * The key/value pair is encoded before it is added to the query.
		 * The key and value are also encoded into UTF-8. Both \em key
		 * and \em value are URL-encoded as well. So, this function is
		 * analogous in effect to standard relevant Qt APIs.
		 *
		 * @param[in] key The query parameter key.
		 * @param[in] value The query parameter value.
		 * @return This UrlOperator object.
		 */
		UrlOperator& operator() (const QString& key, const QString& value);

		/** @brief Adds a new \em key = \em value parameters pair.
		 *
		 * If the URL already contains this \em key, a new value is added
		 * in addition to the already existing one.
		 *
		 * This overload is provided for convenience and efficiency.
		 *
		 * @param[in] key The query parameter key.
		 * @param[in] value The query parameter value.
		 * @return This UrlOperator object.
		 */
		UrlOperator& operator() (const QString& key, const QByteArray& value);

		/** @brief Adds a new \em key = \em value parameters pair.
		 *
		 * If the URL already contains this \em key, a new value is added
		 * in addition to the already existing one.
		 *
		 * The \em value is considered to be a Latin1-string.
		 *
		 * This overload is provided for convenience and efficiency.
		 *
		 * @param[in] key The query parameter key.
		 * @param[in] value The query parameter value (a Latin1-string).
		 * @return This UrlOperator object.
		 */
		UrlOperator& operator() (const QString& key, const char *value);

		/** @brief Adds a new \em key = \em value parameters pair.
		 *
		 * If the URL already contains this \em key, a new value is added
		 * in addition to the already existing one.
		 *
		 * This overload is provided for convenience and efficiency.
		 *
		 * @param[in] key The query parameter key.
		 * @param[in] value The query parameter value.
		 * @return This UrlOperator object.
		 */
		UrlOperator& operator() (const QString& key, int value);

		template<typename Key, typename Value>
		UrlOperator& operator() (bool condition, Key&& key, Value&& value)
		{
			if (condition)
				(*this) (std::forward<Key> (key), std::forward<Value> (value));

			return *this;
		}

		/** @brief Returns the first query parameter under the \em key.
		 *
		 * If no such parameters exist, this function does nothing.
		 *
		 * @param[in] key The query parameter key.
		 * @return This UrlOperator object.
		 */
		UrlOperator& operator-= (const QString& key);

		/** @brief Flushes any pending changes to the QUrl query.
		 */
		QUrl operator() ();
	};
}
}
