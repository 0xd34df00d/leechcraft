/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/media/ihypesprovider.h>
#include "ui_hypeswidget.h"

class QStandardItemModel;

namespace LeechCraft
{
namespace LMP
{
	class HypesWidget : public QWidget
	{
		Q_OBJECT

		Ui::HypesWidget Ui_;
		QStandardItemModel *NewArtistsModel_;
		QStandardItemModel *TopArtistsModel_;
		QStandardItemModel *NewTracksModel_;
		QStandardItemModel *TopTracksModel_;

		QList<QObject*> Providers_;
	public:
		HypesWidget (QWidget* = 0);

		void InitializeProviders ();
	private slots:
		void request ();
		void handleArtists (const QList<Media::HypedArtistInfo>&, Media::IHypesProvider::HypeType);
		void handleTracks (const QList<Media::HypedTrackInfo>&, Media::IHypesProvider::HypeType);
		void handleLink (const QString&);
	};
}
}
