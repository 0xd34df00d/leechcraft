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

namespace LC::LMP
{
	class StdArtistActionsManager : public QObject
	{
		Q_OBJECT
	public:
		explicit StdArtistActionsManager (QQuickWidget& view, QObject *parent = nullptr);

		Q_INVOKABLE void bookmarkArtist (const QString&, const QString&, const QString&);
		Q_INVOKABLE void browseArtistInfo (const QString&);

		Q_INVOKABLE void openLink (const QString&);
	};
}
