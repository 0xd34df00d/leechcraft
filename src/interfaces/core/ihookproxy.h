/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_CORE_IHOOKPROXY_H
#define INTERFACES_CORE_IHOOKPROXY_H
#include <memory>
#include <QMetaType>

class QVariant;
class QByteArray;

namespace LC
{
	/** @brief Class for hook-based communication between plugins.
	 *
	 * This interface is designed to be implemented by classes that
	 * allow plugins to communicate with each other using hooks. Usage
	 * of somewhat standard implementation, Util::DefaultHookProxy, is
	 * encouraged.
	 *
	 * The implementation of this interface may also be considered to be
	 * used as a container for parameters that could be passed to hooks,
	 * modified there and used back in the default handler. For that,
	 * GetValue() and SetValue() members are used.
	 *
	 * Parameters are identified by their names, and the names are
	 * usually documented for each corresponding hook.
	 *
	 * So, a hook should get the current value of the parameter by
	 * calling GetValue(), do the required work and possibly update the
	 * parameter by calling SetValue().
	 *
	 * Please note that if several hooks are connected to a single hook
	 * point, the changes to parameters made by previously called hooks
	 * would be visible to next hooks in chain. That is intentional and
	 * by design.
	 *
	 * It only makes sense to pass parameters like that for objects of
	 * types that are copyable and are usually passed by value or by
	 * reference. For example, that may be standard scalar types (int,
	 * bool), or QString, QByteArray and such.
	 *
	 * The hook may cancel the default handler of an event by calling
	 * CancelDefault().
	 *
	 * @sa Util::DefaultHookProxy
	 */
	class Q_DECL_EXPORT IHookProxy
	{
	public:
		virtual ~IHookProxy () {}

		/** @brief Cancels default handler of the event.
		 *
		 * A canceled event handler can't be uncanceled later.
		 */
		virtual void CancelDefault () = 0;

		/** @brief Returns the current "return value" of this hook call chain.
		 *
		 * @return The current "return value".
		 */
		virtual const QVariant& GetReturnValue () const = 0;

		/** @brief Sets the "return value" of this hook chain.
		 *
		 * Consequent calls to this function would overwrite the
		 * previously set value.
		 *
		 * @param[in] value The new return value of this hook.
		 */
		virtual void SetReturnValue (const QVariant& value) = 0;

		/** @brief Returns the value of the given parameter.
		 *
		 * This function returns current value of the given parameter,
		 * or a null QVariant() if no such parameter has been set.
		 *
		 * @param[in] name The name of the parameter.
		 * @return The parameter's value or null QVariant() if no such
		 * parameter exists.
		 *
		 * @sa SetValue()
		 */
		virtual QVariant GetValue (const QByteArray& name) const = 0;

		/** @brief Updates the value of the given parameter.
		 *
		 * This function sets the value of the parameter identified by
		 * name, possibly overwriting previous value (if any).
		 *
		 * Setting a null QVariant as value effectively erases the
		 * parameter value.
		 *
		 * @param[in] name The name of the parameter.
		 * @param[in] value The new value of the parameter.
		 *
		 * @sa GetValue()
		 */
		virtual void SetValue (const QByteArray& name, const QVariant& value) = 0;
	};

	typedef std::shared_ptr<IHookProxy> IHookProxy_ptr;
}

Q_DECLARE_METATYPE (LC::IHookProxy_ptr)

#endif
