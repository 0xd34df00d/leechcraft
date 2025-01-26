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
#include <QStringList>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Zalil
{
	class ServiceBase;
	typedef std::shared_ptr<ServiceBase> ServiceBase_ptr;

	class PendingUploadBase;

	class ServicesManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		QList<ServiceBase_ptr> Services_;
	public:
		ServicesManager (const ICoreProxy_ptr&, QObject* = 0);

		QStringList GetNames (const QString& file) const;
		PendingUploadBase* Upload (const QString& file, const QString& service);
	signals:
		void fileUploaded (const QString&, const QUrl&);
	};
}
}
