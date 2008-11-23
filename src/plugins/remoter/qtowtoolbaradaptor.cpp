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

