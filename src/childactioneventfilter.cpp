#include "childactioneventfilter.h"
#include <QEvent>
#include <QAction>
#include <QTabWidget>
#include "skinengine.h"

using namespace LeechCraft;

ChildActionEventFilter::ChildActionEventFilter (QObject *parent)
: QObject (parent)
{
}

ChildActionEventFilter::~ChildActionEventFilter ()
{
}

bool ChildActionEventFilter::eventFilter (QObject *obj, QEvent *e)
{
	if (e->type () == QEvent::ChildAdded)
	{
		dynamic_cast<QChildEvent*> (e)->child ()->installEventFilter (this);
		return false;
	}
	else if (e->type () == QEvent::ChildPolished)
	{
		SkinEngine::Instance ()
			.UpdateIconSet (dynamic_cast<QChildEvent*> (e)->child ()->
					findChildren<QAction*> ());
		SkinEngine::Instance ()
			.UpdateIconSet (dynamic_cast<QChildEvent*> (e)->child ()->
					findChildren<QTabWidget*> ());
		return false;
	}
	else
		return QObject::eventFilter (obj, e);
}

