#include "findproxy.h"

FindProxy::FindProxy (const LeechCraft::Request& request,
		QObject *parent)
: QObject (parent)
, ID_ (Core::Instance ().Start (request))
{
}

FindProxy::~FindProxy ()
{
	Core::Instance ().Stop (ID_);
}

