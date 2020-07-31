/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/iloadprogressreporter.h>

namespace LC
{
	class LoadProcessBase : public QObject
						  , public ILoadProcess
	{
		Q_OBJECT
	public:
		virtual QString GetTitle () const = 0;
		virtual int GetMin () const = 0;
		virtual int GetMax () const = 0;
		virtual int GetValue () const = 0;

		void operator++ () override;
	signals:
		void changed ();
	};
}
