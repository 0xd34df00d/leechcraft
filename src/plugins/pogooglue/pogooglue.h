/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/idatafilter.h>

namespace LC
{
namespace Pogooglue
{
	class Plugin : public QObject
				 , public IInfo
				 , public IEntityHandler
				 , public IDataFilter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IEntityHandler IDataFilter)

		LC_PLUGIN_METADATA ("org.LeechCraft.Pogooglue")

		ICoreProxy_ptr Proxy_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		EntityTestHandleResult CouldHandle (const Entity& entity) const;
		void Handle (Entity entity);

		QString GetFilterVerb () const;
		QList<FilterVariant> GetFilterVariants (const QVariant&) const;
	private:
		void GoogleIt (const QString&);
	};
}
}
