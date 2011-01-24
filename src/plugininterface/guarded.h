/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGININTERFACE_GUARDED_H
#define PLUGININTERFACE_GUARDED_H
#include <QReadWriteLock>
#include <QVariant>

namespace LeechCraft
{
	namespace Util
	{
		template<typename T>
		class Guarded
		{
			T Value_;
			QReadWriteLock *Lock_;
		public:
			Guarded ()
			{
				Lock_ = new QReadWriteLock;
			}

			Guarded (const T& val)
			: Value_ (val)
			{
				Lock_ = new QReadWriteLock;
			}

			Guarded (const Guarded& obj)
			: Value_ (obj)
			{
				Lock_ = new QReadWriteLock;
			}

			Guarded& operator= (const Guarded& obj)
			{
				QWriteLocker lock (Lock_);
				Value_ = obj;
				return *this;
			}

			Guarded& operator= (const T& val)
			{
				QWriteLocker lock (Lock_);
				Value_ = val;
				return *this;
			}

			~Guarded ()
			{
				delete Lock_;
			}

			T& Val ()
			{
				QWriteLocker lock (Lock_);
				return Value_;
			}

			const T& Val () const
			{
				QReadLocker lock (Lock_);
				return Value_;
			}

			operator T& ()
			{
				QWriteLocker lock (Lock_);
				return Value_;
			}

			operator const T& () const
			{
				QReadLocker lock (Lock_);
				return Value_;
			}

			operator QVariant () const
			{
				QReadLocker lock (Lock_);
				return qVariantFromValue<T> (Value_);
			}
		};
	};
};

#endif

