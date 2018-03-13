/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <util/sll/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/idatafilter.h>
#include "hostingservice.h"

class QNetworkReply;
class QNetworkAccessManager;
class QStandardItem;
class QStandardItemModel;

namespace LeechCraft
{
struct Entity;

namespace Imgaste
{
	class Poster : public QObject
	{
		Q_OBJECT

		QNetworkReply *Reply_ = nullptr;
		const HostingService Service_;
		const Worker_ptr Worker_;
		const ICoreProxy_ptr Proxy_;
		const DataFilterCallback_f Callback_;

		QStandardItemModel * const ReprModel_;
		const QList<QStandardItem*> ReprRow_;
		const Util::DefaultScopeGuard RowRemoveGuard_;
	public:
		Poster (HostingService service,
				const QByteArray& data,
				const QString& format,
				ICoreProxy_ptr coreProxy,
				DataFilterCallback_f = {},
				QStandardItemModel* = nullptr,
				QObject *parent = nullptr);
	private:
		void HandleReplyFinished (QNetworkReply*);
	};
}
}
