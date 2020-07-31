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
#include <interfaces/ientityhandler.h>
#include <interfaces/idatafilter.h>
#include <interfaces/ijobholder.h>

class QAbstractItemModel;
class QStandardItemModel;

namespace LC::Imgaste
{
	class Plugin : public QObject
				 , public IInfo
				 , public IEntityHandler
				 , public IDataFilter
				 , public IJobHolder
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IEntityHandler IDataFilter IJobHolder)

		LC_PLUGIN_METADATA ("org.LeechCraft.Imgaste")

		ICoreProxy_ptr Proxy_;

		QStandardItemModel *ReprModel_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;

		QString GetFilterVerb () const override;
		QList<FilterVariant> GetFilterVariants (const QVariant&) const override;

		QAbstractItemModel* GetRepresentation () const override;
	private:
		void UploadFile (const QString&, const Entity&);
		void UploadImage (const QImage&, const Entity&);

		void UploadImpl (const QByteArray&, const Entity&, const QString&);
	};
}
