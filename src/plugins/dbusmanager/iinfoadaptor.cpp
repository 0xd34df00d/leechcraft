#include "iinfoadaptor.h"
#include <interfaces/interfaces.h>

IInfoAdaptor::IInfoAdaptor (QObject *info)
: QDBusAbstractAdaptor (info)
, Object_ (qobject_cast<IInfo*> (info))
{
}

