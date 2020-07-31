/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <interfaces/media/iradiostation.h>

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;
class QTimer;
class QUrl;

namespace Media
{
	class IRadioStationProvider;
	struct AudioInfo;
}

namespace LC
{
namespace Util
{
	class MergeModel;
}

namespace LMP
{
	class Player;

	class RadioManager : public QObject
	{
		Q_OBJECT

		Util::MergeModel * const MergeModel_;
		QHash<const QAbstractItemModel*, Media::IRadioStationProvider*> Model2Prov_;

		QTimer *AutoRefreshTimer_;
	public:
		RadioManager (QObject* = 0);

		void InitProviders ();

		QAbstractItemModel* GetModel () const;

		void Refresh (const QModelIndex&);
		void AddUrl (const QModelIndex&, const QUrl&, const QString&);
		void RemoveUrl (const QModelIndex&);
		void Handle (const QModelIndex&, Player*);

		void HandleWokeUp ();

		QList<Media::AudioInfo> GetSources (const QModelIndex&) const;
		QList<Media::AudioInfo> GetSources (const QList<QModelIndex>&) const;

		Media::IRadioStation_ptr GetRadioStation (const QString&) const;
	private:
		void InitProvider (QObject*);

		template<typename F>
		std::result_of_t<F (Media::IRadioStationProvider*, QModelIndex)>
			WithSourceProv (const QModelIndex&, F) const;

		template<typename Succ, typename Fail>
		std::result_of_t<Succ (Media::IRadioStationProvider*, QModelIndex)>
			WithSourceProv (const QModelIndex&, Succ, Fail) const;
	public slots:
		void refreshAll ();
	private slots:
		void handleRefreshSettingsChanged ();
	};
}
}
