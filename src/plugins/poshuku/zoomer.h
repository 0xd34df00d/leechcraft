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

class QWheelEvent;
class QAction;

namespace LC
{
namespace Poshuku
{
	class Zoomer : public QObject
	{
		Q_OBJECT

		const QList<qreal> Zooms_;
	public:
		using ZoomGetter_f = std::function<qreal (void)>;
		using ZoomSetter_f = std::function<void (qreal)>;
	private:
		const ZoomGetter_f Getter_;
		const ZoomSetter_f Setter_;
	public:
		Zoomer (const ZoomGetter_f&,
				const ZoomSetter_f&,
				QObject*,
				const QList<qreal>& = { 0.3, 0.5, 0.67, 0.8, 0.9, 1, 1.1, 1.2, 1.33, 1.5, 1.7, 2, 2.4, 3 });

		void SetActionsTriple (QAction *in, QAction *out, QAction *reset);
		void InstallScrollFilter (QObject*, const std::function<bool (QWheelEvent*)>&);
	private:
		int LevelForZoom (qreal) const;
		void SetZoom (qreal);
	public slots:
		void zoomIn ();
		void zoomOut ();
		void zoomReset ();
	signals:
		void zoomChanged ();
	};
}
}
