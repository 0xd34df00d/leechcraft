/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "qtowtoolbaradaptor.h"
#include <QToolBar>
#include <QtDebug>
#include <QAction>
#include <QImage>
#include <WApplication>
#include <WLogger>
#include <WImage>
#include <WMemoryResource>
#include <WContainerWidget>
#include "util.h"

QToWToolbarAdaptor::QToWToolbarAdaptor (const QToolBar *sourceBar,
		Wt::WContainerWidget *parent)
: Wt::Ext::ToolBar (parent)
{
	QList<QAction*> sourceActions =
		sourceBar->findChildren<QAction*> ();

	for (QList<QAction*>::const_iterator i = sourceActions.begin (),
			end = sourceActions.end (); i != end; ++i)
	{
		std::string text = Util::QStringToUTF8 ((*i)->text ());

		// Seems like we really shouldn't use QPixmaps outside of GUI
		// thread, but here we have no choice.
		QImage tmp = (*i)->icon ().pixmap (32, 32).toImage ()
			.scaled (16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		if (tmp.width () > 0)
		{
			Wt::WMemoryResource *resource = new Wt::WMemoryResource ("image/png",
					Util::PixmapToData (tmp),
					this);

			Wt::WImage *image = new Wt::WImage (resource,
					text,
					parent);
			image->hide ();

			Wt::Ext::Button *button = new Wt::Ext::Button ();
			button->setIcon (image->imageRef ());
			add (button);
		}
		else
			addButton (text);
	}
}

QToWToolbarAdaptor::~QToWToolbarAdaptor ()
{
}

