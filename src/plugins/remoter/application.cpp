#include <WEnvironment>
#include <WString>
#include "application.h"
#include <QtDebug>

Application::Application (const Wt::WEnvironment& env)
: Wt::WApplication (env)
, Ajax_ (env.ajax ())
{
	setTitle (Wt::WString::tr ("LeechCraft::Remoter beta"));
}

Wt::WApplication* ApplicationCreator (const Wt::WEnvironment& env)
{
	qDebug () << Q_FUNC_INFO;
	return new Application (env);
}

