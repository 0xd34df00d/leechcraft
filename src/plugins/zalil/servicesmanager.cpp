/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "servicesmanager.h"
#include <algorithm>
#include <QStringList>
#include <QFileInfo>
#include <QtDebug>
#include "servicebase.h"
#include "pendinguploadbase.h"
#include "bitcheeseservice.h"

namespace LC
{
namespace Zalil
{
	ServicesManager::ServicesManager (const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	{
		Services_ << std::make_shared<BitcheeseService> (proxy, this);
	}

	QStringList ServicesManager::GetNames (const QString& file) const
	{
		const auto fileSize = file.isEmpty () ?
				0 :
				QFileInfo { file }.size ();

		QStringList result;
		for (const auto& service : Services_)
			if (service->GetMaxFileSize () > fileSize)
				result << service->GetName ();

		return result;
	}

	PendingUploadBase* ServicesManager::Upload (const QString& file, const QString& svcName)
	{
		const auto pos = std::find_if (Services_.begin (), Services_.end (),
				[&svcName] (const ServiceBase_ptr& service)
					{ return service->GetName () == svcName; });
		if (pos == Services_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot find service"
					<< svcName;
			return nullptr;
		}

		const auto pending = (*pos)->UploadFile (file);
		if (!pending)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to upload"
					<< file
					<< "to"
					<< svcName;
			return nullptr;
		}

		connect (pending,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SIGNAL (fileUploaded (QString, QUrl)));
		return pending;
	}
}
}
