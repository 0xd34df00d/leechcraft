#include "morphfile.h"

MorphFile::MorphFile (const QString& name)
: QFile (name)
, Gunzip_ (false)
{
}

MorphFile::MorphFile (QObject *parent)
: QFile (parent)
, Gunzip_ (false)
{
}

MorphFile::MorphFile (const QString& name, QObject *parent)
: QFile (name, parent)
, Gunzip_ (false)
{
}

MorphFile::~MorphFile ()
{
}

void MorphFile::Gunzip (bool state)
{
	Gunzip_ = state;
}

