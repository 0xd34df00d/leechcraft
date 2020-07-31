/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iwebfilestorage.h>

class QStandardItemModel;

namespace LC
{
namespace Zalil
{
	class ServicesManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IJobHolder
				 , public IWebFileStorage
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IJobHolder IWebFileStorage)

		LC_PLUGIN_METADATA ("org.LeechCraft.Zalil")

		std::shared_ptr<ServicesManager> Manager_;

		QStandardItemModel* ReprModel_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QAbstractItemModel* GetRepresentation () const;

		QStringList GetServiceVariants () const;
		void UploadFile (const QString&, const QString&);
	signals:
		void fileUploaded (const QString&, const QUrl&);
	};
}
}
