/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <QObject>
#include <QPointer>
#include <QHash>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>

class QUrl;
class QPoint;

namespace LeechCraft
{
namespace SB2
{
	class ViewManager;
	class DeclarativeWindow;
	class QuarkOrderView;

	class QuarkProxy : public QObject
	{
		Q_OBJECT
		Q_PROPERTY (QString extHoveredQuarkClass READ GetExtHoveredQuarkClass NOTIFY extHoveredQuarkClassChanged)

		ViewManager *Manager_;
		ICoreProxy_ptr Proxy_;

		QString ExtHoveredQuarkClass_;

		QHash<QUrl, QPointer<DeclarativeWindow>> URL2LastOpened_;
		QPointer<QuarkOrderView> QuarkOrderView_;
	public:
		QuarkProxy (ViewManager*, ICoreProxy_ptr, QObject* = 0);

		const QString& GetExtHoveredQuarkClass () const;
		QRect GetFreeCoords () const;
	public slots:
		QPoint mapToGlobal (double, double);
		void showTextTooltip (int, int, const QString&);
		void showSettings (const QUrl&);
		void removeQuark (const QUrl&);
		QVariant openWindow (const QUrl&, const QString&, const QVariant&);
		QRect getWinRect ();

		void quarkAddRequested (int, int);
		void quarkOrderRequested (int, int);
	private slots:
		void handleExtHoveredQuarkClass (const QString&);
	signals:
		void extHoveredQuarkClassChanged ();
	};
}
}
