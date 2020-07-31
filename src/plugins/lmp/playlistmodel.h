/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <util/models/dndactionsmixin.h>

namespace LC
{
namespace LMP
{
	class Player;

	class PlaylistModel : public Util::DndActionsMixin<QStandardItemModel>
	{
		Player * const Player_;
	public:
		PlaylistModel (Player*);

		QStringList mimeTypes () const;
		QMimeData* mimeData (const QModelIndexList&) const;
		bool dropMimeData (const QMimeData*, Qt::DropAction, int, int, const QModelIndex&);
		Qt::DropActions supportedDropActions () const;
	private:
		void HandleRadios (const QMimeData*);
		void HandleDroppedUrls (const QMimeData*, int row, const QModelIndex& parent);
	};
}
}
