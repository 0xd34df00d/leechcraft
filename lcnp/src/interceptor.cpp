#include "interceptor.h"
#include <QHash>

Interceptor::Interceptor (QObject *parent)
: QObject (parent)
{
}

Interceptor::~Interceptor ()
{
}

bool Interceptor::readData (QIODevice *source, const QString& format)
{
	Data_ [source] = source->readAll ();

	if (!source->atEnd ())
		connect (source,
				SIGNAL (readChannelFinished ()),
				this,
				SLOT (handleReadChannelFinished ()));

	return true;
}

void Interceptor::handleReadChannelFinished ()
{
	QIODevice *source = static_cast<QIODevice*> (sender ());
	Data_ [source] += source->readAll ();

	Data_.remove (source);
}

QTNPFACTORY_BEGIN ("LC Interceptor",
		"A Qt-based plugin that intercepts download jobs and passes them to LeechCraft")
	QTNPCLASS (Interceptor)
QTNPFACTORY_END()

