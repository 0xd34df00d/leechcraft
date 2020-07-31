/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

namespace LC
{
namespace Snails
{
	class AccountThreadNotifierBase : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		void changed ();
	};

	template<typename T>
	class AccountThreadNotifier : public AccountThreadNotifierBase
	{
		mutable QReadWriteLock Lock_;

		T Data_;
	public:
		using AccountThreadNotifierBase::AccountThreadNotifierBase;

		void SetData (const T& t)
		{
			if (GetData () == t)
				return;

			{
				QWriteLocker locker { &Lock_ };
				Data_ = t;
			}

			emit changed ();
		}

		T GetData () const
		{
			QReadLocker locker { &Lock_ };
			return Data_;
		}
	};
}
}
