#include <QtPlugin>
#include "batcher.h"
#include "globals.h"

void Batcher::Init ()
{
}

QString Batcher::GetName () const
{
	return Globals::Name;
}

QString Batcher::GetInfo () const
{
	return tr ("Batch job adder.");
}

QString Batcher::GetStatusbarMessage () const
{
	return "";
}

IInfo& Batcher::SetID (IInfo::ID_t id)
{
	ID_ = id;
}

IInfo::ID_t Batcher::GetID () const
{
	return ID_;
}

QStringList Batcher::Provides () const
{
	return QStringList ();
}

QStringList Batcher::Needs () const
{
	return QStringList ("http") << "ftp";
}

QStringList Batcher::Uses () const
{
	return QStringList ();
}

Q_EXPORT_PLUGIN2 (leechcraft_batcher, Batcher);
