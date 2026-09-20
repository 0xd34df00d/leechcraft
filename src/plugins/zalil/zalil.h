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
#include <util/xpc/progressmanager.h>
#include <interfaces/iinfo.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iwebfilestorage.h>

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

		mutable Util::ProgressManager Progress_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		IJobHolderRepresentationHandler_ptr CreateRepresentationHandler (const ViewCallbacks&) override;

		QStringList GetServiceVariants () const override;
		void UploadFile (const QString&, const QString&) override;
	signals:
		void fileUploaded (const QString&, const QUrl&) override;
	};
}
}
