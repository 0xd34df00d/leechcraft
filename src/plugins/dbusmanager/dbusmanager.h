/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/iinfo.h>

namespace LC
{
namespace DBusManager
{
	class DBusManager : public QObject
						, public IInfo
	{
		Q_OBJECT
		Q_INTERFACES (IInfo)

		LC_PLUGIN_METADATA ("org.LeechCraft.DBusManager")
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QStringList Provides () const;
		QIcon GetIcon () const;
	};
}
}
