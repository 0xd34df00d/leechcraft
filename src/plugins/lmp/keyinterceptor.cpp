#include "keyinterceptor.h"
#include <QKeyEvent>
#include "core.h"

KeyInterceptor::KeyInterceptor (QObject *parent)
: QObject (parent)
{
}

KeyInterceptor::~KeyInterceptor ()
{
}

bool KeyInterceptor::eventFilter (QObject *obj, QEvent *e)
{
	if (e->type () == QEvent::KeyPress)
	{
		keyPressEvent (dynamic_cast<QKeyEvent*> (e));
		return true;
	}
	else if (e->type () == QEvent::KeyRelease)
	{
		keyReleaseEvent (dynamic_cast<QKeyEvent*> (e));
		return true;
	}
	return QObject::eventFilter (obj, e);
}

void KeyInterceptor::keyPressEvent (QKeyEvent *e)
{
	if (e->text () == "*")
		Core::Instance ().IncrementVolume ();
	else if (e->text () == "/")
		Core::Instance ().DecrementVolume ();
	else if (e->key () == Qt::Key_Right)
		Core::Instance ().Forward (Core::SkipLittle);
	else if (e->key () == Qt::Key_Left)
		Core::Instance ().Rewind (Core::SkipLittle);
	else if (e->key () == Qt::Key_Up)
		Core::Instance ().Forward (Core::SkipMedium);
	else if (e->key () == Qt::Key_Down)
		Core::Instance ().Rewind (Core::SkipMedium);
	else if (e->key () == Qt::Key_PageUp)
		Core::Instance ().Forward (Core::SkipALot);
	else if (e->key () == Qt::Key_PageDown)
		Core::Instance ().Rewind (Core::SkipALot);
}

void KeyInterceptor::keyReleaseEvent (QKeyEvent *e)
{
	if (e->key () == Qt::Key_Return ||
			e->key () == Qt::Key_Enter)
		Core::Instance ().ToggleFullScreen ();
	else if (e->key () == Qt::Key_Space)
		Core::Instance ().TogglePause ();
}

