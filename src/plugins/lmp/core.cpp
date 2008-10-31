#include "core.h"
#include <MediaObject>

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

Phonon::MediaObject* Core::CreateObject (const QString& entity)
{
	Phonon::MediaObject *result = new Phonon::MediaObject (this);
}

