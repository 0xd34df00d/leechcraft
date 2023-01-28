/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsView>
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
		QSize SSize_;
	public:
		explicit GlanceShower (ICoreTabWidget&, QWidget* = nullptr);

		void Start ();
	private:
		void Finalize ();
	protected:
		void keyPressEvent (QKeyEvent*) override;
		void mousePressEvent (QMouseEvent *) override;
	private slots:
		void handleSelected (int);
		void handleClosed (int);
	signals:
		void finished ();
	};
}
