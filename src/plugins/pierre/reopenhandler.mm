#include "reopenhandler.h"
#include <QDebug>
#import <AppKit/NSApplication.h>
#import <objc/runtime.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/imwproxy.h>

namespace LC::Pierre
{
	ReopenHandler::ReopenHandler ()
	{
		Class cls = [[[NSApplication sharedApplication] delegate] class];

		SEL sel = @selector (applicationShouldHandleReopen:hasVisibleWindows:);
		Method m0 = class_getInstanceMethod (cls, sel);
		const auto handler = [] (id, SEL) { ReopenHandler::Instance ().Triggered (); };
		if (!class_addMethod (cls, sel, (IMP) +handler, method_getTypeEncoding(m0)))
			qWarning () << Q_FUNC_INFO << "class_addMethod() failed.";
	}

	ReopenHandler& ReopenHandler::Instance ()
	{
		static ReopenHandler instance;
		return instance;
	}

	void ReopenHandler::Triggered ()
	{
		const auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();

		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
		{
			const auto mwProxy = rootWM->GetMWProxy (i);
			mwProxy->ShowMain ();
		}
	}
}
