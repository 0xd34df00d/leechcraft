/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef UTIL_DEFAULTHOOKPROXY_H
#define UTIL_DEFAULTHOOKPROXY_H
#include <QMap>
#include "interfaces/iinfo.h"
#include "interfaces/core/ihookproxy.h"
#include "utilconfig.h"

namespace LeechCraft
{
	namespace Util
	{
		class UTIL_API DefaultHookProxy : public IHookProxy
		{
			bool Cancelled_;
			QVariant ReturnValue_;

			QMap<QByteArray, QVariant> Name2NewVal_;
		public:
			DefaultHookProxy ();

			void CancelDefault ();
			bool IsCancelled () const;
			const QVariant& GetReturnValue () const;
			void SetReturnValue (const QVariant&);

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

			QVariant GetValue (const QByteArray&) const;

			void SetValue (const QByteArray&, const QVariant&);
		};

		typedef std::shared_ptr<DefaultHookProxy> DefaultHookProxy_ptr;
	};
};

#endif
