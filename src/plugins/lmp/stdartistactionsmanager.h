/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QQuickWidget;

namespace LC
{
namespace LMP
{
	class StdArtistActionsManager : public QObject
	{
		Q_OBJECT
	public:
		StdArtistActionsManager (QQuickWidget *view, QObject *parent = 0);
	private slots:
		void handleBookmark (const QString&, const QString&, const QString&);
		void handleLink (const QString&);
	};
}
}
