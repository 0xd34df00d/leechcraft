#include "core.h"

Core::Core ()
{
}

Core& Core::Instance ()
{
    static Core core;
    return core;
}

void Core::Release ()
{
}

void Core::SetProvider (QObject *provider, const QString& feature)
{
}

