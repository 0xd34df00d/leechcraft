#include <QtDebug>
#include "impbase.h"

ImpBase::ImpBase (QObject *parent)
: QThread (parent)
, CacheSize_ (0)
{
}

ImpBase::~ImpBase ()
{
}

void ImpBase::StartDownload ()
{
    start ();
}

void ImpBase::SetCacheSize (int cs)
{
    CacheSize_ = cs;
    Cache_.reserve (CacheSize_);
}

void ImpBase::Emit (ImpBase::length_t down, ImpBase::length_t total, QByteArray data)
{
    emit dataFetched (down, total, data);
}

void ImpBase::EmitFlush (ImpBase::length_t down, ImpBase::length_t total)
{
    emit dataFetched (down, total, QByteArray ());
}

