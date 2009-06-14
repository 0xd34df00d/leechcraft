#include "iinfoadaptor.h"
#include <interfaces/iinfo.h>

using namespace LeechCract::Plugins::DBusManager;

IInfoAdaptor::IInfoAdaptor (QObject *info)
: QDBusAbstractAdaptor (info)
, Object_ (qobject_cast<IInfo*> (info))
{
}

