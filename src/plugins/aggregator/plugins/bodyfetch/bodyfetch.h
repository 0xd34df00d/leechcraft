/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include <QDir>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/aggregator/iaggregatorplugin.h>
#include <interfaces/aggregator/item.h>
#include <interfaces/core/ihookproxy.h>

namespace LC
{
namespace Aggregator
{
class IProxyObject;

struct Item;
using Item_cptr = std::shared_ptr<const Item>;

namespace BodyFetch
{
	class WorkerObject;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IAggregatorPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				LC::Aggregator::IAggregatorPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Aggregator.BodyFetch")

		ICoreProxy_ptr Proxy_;
		QDir StorageDir_;
		WorkerObject *WO_ = nullptr;
		QHash<int, QString> ContentsCache_;
		QSet<quint64> FetchedItems_;

		IProxyObject *AggregatorProxy_ = nullptr;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		void InitPlugin (IProxyObject*) override;
	public slots:
		void hookItemLoad (LC::IHookProxy_ptr proxy,
				Item*);
		void hookItemAdded (LC::IHookProxy_ptr proxy,
				const Item& item);
	private slots:
		void handleDownload (QUrl);
		void handleBodyFetched (quint64);
	signals:
		void downloadFinished (QUrl, QString);
	};
}
}
}
