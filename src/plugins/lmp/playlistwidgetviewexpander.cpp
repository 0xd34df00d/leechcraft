/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playlistwidgetviewexpander.h"
#include <QAbstractItemModel>
#include <util/sll/delayedexecutor.h>

namespace LC
{
namespace LMP
{
	PlaylistWidgetViewExpander::PlaylistWidgetViewExpander (QAbstractItemModel *model,
			const std::function<void ()>& expander,
			QObject *parent)
	: QObject { parent }
	, Expander_ { expander }
	{
		connect (model,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (checkRowInsertion ()));
	}

	void PlaylistWidgetViewExpander::checkRowInsertion ()
	{
		if (IsScheduled_)
			return;

		IsScheduled_ = true;

		Util::ExecuteLater ([this]
				{
					IsScheduled_ = false;
					Expander_ ();
				});
	}
}
}
