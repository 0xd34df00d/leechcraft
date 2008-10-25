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
}

QString Adaptor::GetOrganizationName () const
{
	return Proxy::Instance ()->GetOrganizationName ();
}

QString Adaptor::GetApplicationName () const
{
	return Proxy::Instance ()->GetApplicationName ();
}

void Adaptor::Greeter (const QString& msg)
{
	Core_->Greeter (msg);
}

