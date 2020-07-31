/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/media/iradiostationprovider.h>

class QUrl;
class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace LMP
{
	class RadioCustomStreams : public QObject
							 , public Media::IRadioStationProvider
	{
		Q_OBJECT
		Q_INTERFACES (Media::IRadioStationProvider)

		QStandardItemModel * const Model_;
		QStandardItem * const Root_;
	public:
		RadioCustomStreams (QObject* = 0);

		QList<QAbstractItemModel*> GetRadioListItems () const;
		Media::IRadioStation_ptr GetRadioStation (const QModelIndex&, const QString&);
		void RefreshItems (const QList<QModelIndex>&);

		void Add (const QUrl&, const QString&);
		void Remove (const QModelIndex&);
	private:
		void CreateItem (const QUrl&, const QString&);
		QList<QUrl> GetAllUrls () const;

		void LoadSettings ();
		void SaveSettings () const;
	};
}
}
