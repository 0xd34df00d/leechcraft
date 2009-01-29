#ifndef DEBUGMESSAGEHANDLER_H
#define DEBUGMESSAGEHANDLER_H
#include <QtGlobal>
#include <QMutex>

namespace DebugHandler
{
	static bool PrintStack_ = false;
	void debugMessageHandler (QtMsgType type, const char *message);
};

#endif

