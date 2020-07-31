/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>

namespace LC
{
namespace NamAuth
{
	class Plugin : public QObject
				 , public IInfo
	{
		Q_OBJECT
		Q_INTERFACES (IInfo)

		LC_PLUGIN_METADATA ("org.LeechCraft.NamAuth")
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
	private:
		void InitStorage (const ICoreProxy_ptr&);
	};
}
}
