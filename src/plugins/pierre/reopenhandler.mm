#include "reopenhandler.h"
#include <QMetaObject>
#include <QPointer>
#include <AppKit/NSApplication.h>
#import <objc/runtime.h>

namespace
{
static QPointer<QObject> gPlugin;

static void dockClickHandler(id self, SEL _cmd)
{
	Q_UNUSED (self);
	Q_UNUSED (_cmd);

	if (!gPlugin)
	{
		return;
	}
	QMetaObject::invokeMethod (gPlugin.data (),
							   "reopenRequested",
							   Qt::QueuedConnection);
}

}

namespace LeechCraft
{
namespace Pierre
{
namespace RH
{

bool InitReopenHandler (QObject* pPlugin)
{
	Class cls = [[[NSApplication sharedApplication] delegate] class];

	static bool methodAdded;
	if (!methodAdded)
	{
		SEL sel = @selector (applicationShouldHandleReopen:hasVisibleWindows:);
		Method m0 = class_getInstanceMethod (cls, sel);
		if (!class_addMethod (cls, sel, (IMP) dockClickHandler, method_getTypeEncoding(m0)))
			return false;
		methodAdded = true;
	}

	gPlugin = pPlugin;
	return true;
}

void Shutdown ()
{
	gPlugin = nullptr;
}

}
}
}
