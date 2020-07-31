/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ientityhandler.h>

class QModelIndex;

namespace LC
{
namespace HistoryHolder
{
	class HistoryDB;

	class Plugin : public QObject
					, public IInfo
					, public IFinder
					, public IEntityHandler
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IFinder IEntityHandler)

		LC_PLUGIN_METADATA ("org.LeechCraft.HistoryHolder")

		std::shared_ptr<HistoryDB> DB_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QStringList Provides () const;

		QStringList GetCategories () const;
		QList<IFindProxy_ptr> GetProxy (const Request&);

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);
	signals:
		void categoriesChanged (const QStringList&, const QStringList&);
	};
}
}
