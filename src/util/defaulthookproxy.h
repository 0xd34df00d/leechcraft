/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <QMap>
#include "interfaces/iinfo.h"
#include "interfaces/core/ihookproxy.h"
#include "utilconfig.h"

namespace LeechCraft
{
namespace Util
{
	/** @brief Standard implementation of IHookProxy.
	 *
	 * This class is the standard implementation of the IHookProxy
	 * interface and can be used in most cases.
	 *
	 * @sa IHookProxy
	 */
	class UTIL_API DefaultHookProxy : public IHookProxy
	{
		bool Cancelled_;
		QVariant ReturnValue_;

		QMap<QByteArray, QVariant> Name2NewVal_;
	public:
		/** @brief Creates a new hook proxy.
		 */
		DefaultHookProxy ();

		/** @brief Reimplemented from IHookProxy::CancelDefault().
		 *
		 * @sa IsCancelled()
		 */
		void CancelDefault ();

		/** @brief Returns whether the default implementation is canceled.
		 *
		 * This function returns whether CancelDefault() has been called
		 * at least once.
		 *
		 * @return Whether the default implementation is canceled.
		 *
		 * @sa CancelDefault()
		 */
		bool IsCancelled () const;

		/** @brief Reimplemented from IHookProxy::GetReturnValue().
		 */
		const QVariant& GetReturnValue () const;

		/** @brief Reimplemented from IHookProxy::SetReturnValue().
		 */
		void SetReturnValue (const QVariant&);

		/** @brief Fills the value of the given parameter set by SetValue().
		 *
		 * If SetValue() has been called with the given parameter
		 * \em name this function sets \em val to that value, otherwise
		 * it does nothing.
		 *
		 * @param[in] name The name of the parameter.
		 * @param[out] val The value to fill.
		 * @tparam T The type of the value, which should be known to Qt's
		 * metatypes system so that <code>QVariant::value<T>()</code> is valid.
		 */
		template<typename T>
		void FillValue (const QByteArray& name, T& val)
		{
			if (!Name2NewVal_.contains (name))
				return;

			const QVariant& newVal = Name2NewVal_ [name];
			if (!newVal.isValid ())
				return;

			val = newVal.value<T> ();
		}

		/** @brief Reimplemented from IHookProxy::GetValue().
		 */
		QVariant GetValue (const QByteArray&) const;

		/** @brief Reimplemented from IHookProxy::SetValue().
		 */
		void SetValue (const QByteArray&, const QVariant&);
	};

	typedef std::shared_ptr<DefaultHookProxy> DefaultHookProxy_ptr;
}
}
