/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <QPointer>
#include <QColor>

namespace LC
{
namespace Lemon
{
	class TrafficManager;
	class TrafficDialog;

	class QuarkProxy : public QObject
	{
		Q_OBJECT

		TrafficManager * const TrafficMgr_;

		QMap<QString, QPointer<TrafficDialog>> Iface2Dialog_;

		Q_PROPERTY (QColor downloadGraphColor READ GetDownloadGraphColor NOTIFY downloadGraphColorChanged);
		Q_PROPERTY (QColor uploadGraphColor READ GetUploadGraphColor NOTIFY uploadGraphColorChanged);
	public:
		QuarkProxy (TrafficManager*, QObject* = nullptr);

		QColor GetDownloadGraphColor () const;
		QColor GetUploadGraphColor () const;
	public slots:
		void showGraph (const QString&);
	signals:
		void downloadGraphColorChanged ();
		void uploadGraphColorChanged ();
	};
}
}
