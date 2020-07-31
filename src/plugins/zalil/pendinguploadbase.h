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
#include <QUrl>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/util.h>

class QStandardItem;
class QStandardItemModel;
class QHttpMultiPart;

namespace LC
{
namespace Zalil
{
	class PendingUploadBase : public QObject
	{
		Q_OBJECT

		const QList<QStandardItem*> ProgressRow_;
		const Util::DefaultScopeGuard ProgressRowGuard_;
	protected:
		const QString Filename_;
		const ICoreProxy_ptr Proxy_;
	public:
		PendingUploadBase (const QString& filename, const ICoreProxy_ptr&, QObject* = nullptr);

		const QList<QStandardItem*>& GetReprRow () const;
	protected:
		QHttpMultiPart* MakeStandardMultipart ();
	protected slots:
		void handleUploadProgress (qint64, qint64);
		virtual void handleError ();
		virtual void handleFinished () = 0;
	signals:
		void fileUploaded (const QString&, const QUrl&);
	};
}
}
