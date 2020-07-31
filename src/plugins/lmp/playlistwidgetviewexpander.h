/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>

class QAbstractItemModel;
class QModelIndex;

namespace LC
{
namespace LMP
{
	class PlaylistWidgetViewExpander : public QObject
	{
		Q_OBJECT

		std::function<void ()> Expander_;
		bool IsScheduled_ = false;
	public:
		PlaylistWidgetViewExpander (QAbstractItemModel*, const std::function<void ()>&, QObject*);
	private slots:
		void checkRowInsertion ();
	};
}
}
