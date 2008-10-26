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

QString Adaptor::Greeter (const QString& msg,
		const QDBusMessage&,
		QString& reply)
{
	Core_->Greeter (msg);
	reply = "shit";
	return "Reply from LeechCraft!";
}

