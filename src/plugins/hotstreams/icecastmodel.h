/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QIcon>

namespace LC
{
namespace HotStreams
{
	class IcecastModel : public QAbstractItemModel
	{
		const QIcon RadioIcon_ { ":/hotstreams/resources/images/radio.png" };
	public:
		struct StationInfo
		{
			QString Name_;
			QString Genre_;
			int Bitrate_ = 0;
			QList<QUrl> URLs_;
			QString MIME_;

			friend bool operator== (const StationInfo&, const StationInfo&);
		};

		using StationInfoList_t = QList<std::pair<QString, QList<StationInfo>>>;
	private:
		StationInfoList_t Stations_;
	public:
		using QAbstractItemModel::QAbstractItemModel;

		QModelIndex index (int row, int column, const QModelIndex& parent = {}) const override;
		QModelIndex parent (const QModelIndex& child) const override;
		int rowCount (const QModelIndex& parent = {}) const override;
		int columnCount (const QModelIndex& parent = {}) const override;
		Qt::ItemFlags flags (const QModelIndex& index) const override;
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;

		void SetStations (const StationInfoList_t& stations);
	private:
		QVariant GetStationData (const QModelIndex&, int role) const;
	};
}
}
