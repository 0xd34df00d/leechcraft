#include "adaptor.h"
#include <plugininterface/proxy.h>
#include "core.h"

Adaptor::Adaptor (Core *parent)
: QDBusAbstractAdaptor (parent)
, Core_ (parent)
{
	connect (parent,
			SIGNAL (aboutToQuit ()),
			this,
			SIGNAL (aboutToQuit ()));
	connect (parent,
			SIGNAL (someEventHappened (const QString&)),
			this,
			SIGNAL (someEventHappened (const QString&)));
}

QString Adaptor::GetOrganizationName () const
{
	return Proxy::Instance ()->GetOrganizationName ();
}

QString Adaptor::GetApplicationName () const
{
	return Proxy::Instance ()->GetApplicationName ();
}

QString Adaptor::Greeter (const QString& msg,
		const QDBusMessage&)
{
	return Core_->Greeter (msg);
}

QStringList Adaptor::GetLoadedPlugins ()
{
	return Core_->GetLoadedPlugins ();
}

