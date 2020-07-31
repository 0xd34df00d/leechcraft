/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_OTZERKALU_OTZERKALU_H
#define PLUGINS_OTZERKALU_OTZERKALU_H
#include <QObject>
#include <QUrl>
#include <QStandardItemModel>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include "otzerkaludownloader.h"

namespace LC
{
namespace Otzerkalu
{
	class Plugin : public QObject
				 , public IInfo
				 , public IEntityHandler
				 , public IJobHolder
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IEntityHandler IJobHolder)

		LC_PLUGIN_METADATA ("org.LeechCraft.Otzerkalu")

		QStandardItemModel *RepresentationModel_;
		ICoreProxy_ptr Proxy_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;
		EntityTestHandleResult CouldHandle (const Entity& entity) const override;
		void Handle (Entity entity) override;
		QAbstractItemModel* GetRepresentation () const override;
	};
}
}

#endif

