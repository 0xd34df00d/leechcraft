/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsView>
#include <QVector>
#include <interfaces/iinfo.h>

class ICoreTabWidget;

namespace LC::Plugins::Glance
{
	class GlanceItem;

	class GlanceShower : public QGraphicsView
	{
		Q_OBJECT

		ICoreTabWidget& TabWidget_;
		QGraphicsScene *Scene_;

		QVector<GlanceItem*> Items_;
	public:
		explicit GlanceShower (ICoreTabWidget&, QWidget* = nullptr);

		void Start ();
	private:
		void Finalize ();
		void HandleSelected (int);
		void HandleClosed (int);

		void Reposition ();
	protected:
		void keyPressEvent (QKeyEvent*) override;
		void mousePressEvent (QMouseEvent *) override;
	signals:
		void finished ();
	};
}
