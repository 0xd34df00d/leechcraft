/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QWebSettings;

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class SettingsInstanceHandler : public QObject
	{
		Q_OBJECT

		QWebSettings * const Settings_;
	public:
		SettingsInstanceHandler (QWebSettings*, QObject* = nullptr);
	private slots:
		void generalSettingsChanged ();
		void cacheSettingsChanged ();
		void setUserStyleSheet ();
	};
}
}
}
