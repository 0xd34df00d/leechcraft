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
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Zalil
{
	class PendingUploadBase;

	class ServiceBase : public QObject
	{
		Q_OBJECT
	protected:
		const ICoreProxy_ptr Proxy_;
	public:
		ServiceBase (const ICoreProxy_ptr&, QObject* = nullptr);

		virtual QString GetName () const = 0;
		virtual qint64 GetMaxFileSize () const = 0;
		virtual PendingUploadBase* UploadFile (const QString&) = 0;
	};

	typedef std::shared_ptr<ServiceBase> ServiceBase_ptr;
}
}
