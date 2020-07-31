/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QHash>
#include <interfaces/iinfo.h>
#include <interfaces/media/iradiostationprovider.h>

class QStandardItem;
class QStandardItemModel;

namespace LC
{
namespace HotStreams
{
	class IcecastModel;

	class Plugin : public QObject
				 , public IInfo
				 , public Media::IRadioStationProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo Media::IRadioStationProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.HotStreams")

		ICoreProxy_ptr Proxy_;

		IcecastModel *IcecastModel_;

		QStandardItemModel *Model_;
		QHash<QStandardItem*, std::function<void (QStandardItem*)>> Root2Fetcher_;
		QHash<const QAbstractItemModel*, std::function<void ()>> Model2Fetcher_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QList<QAbstractItemModel*> GetRadioListItems () const;
		Media::IRadioStation_ptr GetRadioStation (const QModelIndex&, const QString&);
		void RefreshItems (const QList<QModelIndex>&);
	};
}
}

