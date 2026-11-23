/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QSplashScreen>

namespace LC
{
	class LoadProcessBase;

	class SplashScreen : public QSplashScreen
	{
		Q_DECLARE_TR_FUNCTIONS (LC::SplashScreen)

		QList<LoadProcessBase*> Processes_;
	public:
		using QSplashScreen::QSplashScreen;

		void RegisterLoadProcess (LoadProcessBase*);
	protected:
		void drawContents (QPainter*) override;
	};
}
