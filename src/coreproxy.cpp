#include "coreproxy.h"
#include "core.h"

using namespace LeechCraft;

CoreProxy::CoreProxy (QObject *parent)
: QObject (parent)
{
}

QNetworkAccessManager* CoreProxy::GetNetworkAccessManager () const
{
	return Core::Instance ().GetNetworkAccessManager ();
}

const IShortcutProxy* CoreProxy::GetShortcutProxy () const
{
	return Core::Instance ().GetShortcutProxy ();
}

