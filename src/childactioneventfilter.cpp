#include "childactioneventfilter.h"
#include <QEvent>
#include "skinengine.h"

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
		dynamic_cast<QChildEvent*> (e)->child ()->installEventFilter (this);
	else if (e->type () == QEvent::ChildPolished)
		Main::SkinEngine::Instance ()
			.updateIconSet (dynamic_cast<QChildEvent*> (e)->child ()->
					findChildren<QAction*> ());
	return QObject::eventFilter (obj, e);
}

