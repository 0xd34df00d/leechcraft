/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_GLANCE_GLANCESHOWER_H
#define PLUGINS_GLANCE_GLANCESHOWER_H
#include <QGraphicsView>
#include <interfaces/iinfo.h>

class ICoreTabWidget;

namespace LC
{
namespace Plugins
{
namespace Glance
{
	class GlanceItem;

	class GlanceShower : public QGraphicsView
	{
		Q_OBJECT

		ICoreTabWidget *TabWidget_ = nullptr;
		QGraphicsScene *Scene_;
		bool Shown_ = false;
		QSize SSize_;
	public:
		GlanceShower (QWidget* = 0);
		void SetTabWidget (ICoreTabWidget*);
		void Start ();
	private:
		void Finalize ();
	protected:
		void keyPressEvent (QKeyEvent*);
		void mousePressEvent (QMouseEvent *);
	private slots:
		void handleClicked (int, bool = false);
	signals:
		void finished (bool);
	};
};
};
};

#endif

