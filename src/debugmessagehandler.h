#ifndef DEBUGMESSAGEHANDLER_H
#define DEBUGMESSAGEHANDLER_H
#include <QtGlobal>
#include <QMutex>

namespace DebugHandler
{
	void simple (QtMsgType type, const char *message);
	void backtraced (QtMsgType type, const char *message);
};

#endif

