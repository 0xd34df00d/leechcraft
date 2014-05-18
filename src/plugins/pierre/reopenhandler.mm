#include "reopenhandler.h"
#include <QDebug>
#import <AppKit/NSApplication.h>
#import <objc/runtime.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/imwproxy.h>


namespace
{
	static void dockClickHandler(id, SEL)
	{
		LeechCraft::Pierre::ReopenHandler::Instance ().Triggered ();
	}
}

namespace LeechCraft
{
namespace Pierre
{
	ReopenHandler::ReopenHandler ()
	{
		Class cls = [[[NSApplication sharedApplication] delegate] class];

		SEL sel = @selector (applicationShouldHandleReopen:hasVisibleWindows:);
		Method m0 = class_getInstanceMethod (cls, sel);
		if (!class_addMethod (cls, sel, (IMP) dockClickHandler, method_getTypeEncoding(m0)))
			qWarning () << Q_FUNC_INFO << "class_addMethod() failed.";
	}

	ReopenHandler& ReopenHandler::Instance ()
	{
		static ReopenHandler instance;
		return instance;
	}

	void ReopenHandler::SetCoreProxy (const ICoreProxy_ptr& proxy)
	{
		Proxy_ = proxy;
	}

	void ReopenHandler::Triggered ()
	{
		IRootWindowsManager * const rootWM = Proxy_->GetRootWindowsManager ();

		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
		{
			IMWProxy * const mwProxy = rootWM->GetMWProxy (i);
			mwProxy->ShowMain ();
		}
	}
}
}
