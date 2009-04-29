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

