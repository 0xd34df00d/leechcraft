/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <QHash>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>

class QPoint;

namespace LC::SB2
{
	class ViewManager;
	class DeclarativeWindow;
	class QuarkOrderView;

	class QuarkProxy : public QObject
	{
		Q_OBJECT
		Q_PROPERTY (QString extHoveredQuarkClass READ GetExtHoveredQuarkClass NOTIFY extHoveredQuarkClassChanged)

		ViewManager *Manager_;

		QString ExtHoveredQuarkClass_;

		QHash<QUrl, QPointer<DeclarativeWindow>> URL2LastOpened_;
		QPointer<QuarkOrderView> QuarkOrderView_;
	public:
		QuarkProxy (ViewManager*, QObject* = nullptr);

		const QString& GetExtHoveredQuarkClass () const;
		QRect GetFreeCoords () const;
	public slots:
		QPoint mapToGlobal (double, double);
		void showTextTooltip (int, int, const QString&);
		void removeQuark (const QUrl&);
		QRect getWinRect ();

		QPoint fitRect (const QPoint& src, const QSize& size, const QRect& rect, bool canOverlap);
		void registerAutoresize (const QPoint&, const QVariant&);

		void panelMoveRequested (const QString&);

		void quarkAddRequested (int, int);
		void quarkOrderRequested (int, int);

		void panelSettingsRequested ();

		QString prettySize (qint64);
		QString prettySizeShort (qint64);
		QString prettyTime (qint64);

		// internal
		void instantiateQuark (const QUrl& sourceUrl, QObject *obj);
	signals:
		void extHoveredQuarkClassChanged ();
	};
}
