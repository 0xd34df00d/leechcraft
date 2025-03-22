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

namespace LC::NamAuth
{
	class Plugin : public QObject
				 , public IInfo
	{
		Q_OBJECT
		Q_INTERFACES (IInfo)

		LC_PLUGIN_METADATA ("org.LeechCraft.NamAuth")
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;
	private:
		void InitStorage (const ICoreProxy_ptr&);
	};
}
