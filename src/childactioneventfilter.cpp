#include "childactioneventfilter.h"
#include <QEvent>
#include <QAction>
#include <QTabWidget>
#include <QActionEvent>
#include <QtDebug>
#include "skinengine.h"

using namespace LeechCraft;

ChildActionEventFilter::ChildActionEventFilter (QObject *parent)
: QObject (parent)
{
}

ChildActionEventFilter::~ChildActionEventFilter ()
{
}

bool ChildActionEventFilter::eventFilter (QObject *obj, QEvent *event)
{
	if (event->type () == QEvent::ChildAdded ||
			event->type () == QEvent::ChildPolished)
	{
		QChildEvent *e = dynamic_cast<QChildEvent*> (event);
		e->child ()->installEventFilter (this);

		QAction *act = dynamic_cast<QAction*> (e->child ());
		if (act)
			SkinEngine::Instance ().UpdateIconSet (QList<QAction*> () << act);
		else
		{
			SkinEngine::Instance ()
				.UpdateIconSet (e->child ()->findChildren<QAction*> ());
			SkinEngine::Instance ()
				.UpdateIconSet (e->child ()->findChildren<QTabWidget*> ());
		}
		return false;
	}
	else
		return QObject::eventFilter (obj, event);
}

