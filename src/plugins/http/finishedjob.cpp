#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <exceptions/invalidparameter.h>
#include <plugininterface/proxy.h>
#include "finishedjob.h"
#include "jobparams.h"
#include "jobrepresentation.h"

FinishedJob::FinishedJob ()
{
}

const QString& FinishedJob::GetURL () const
{
    return URL_;
}

const QString& FinishedJob::GetLocal () const
{
    return Local_;
}

ImpBase::length_t FinishedJob::GetSize () const
{
    return Size_;
}

const QString& FinishedJob::GetSpeed () const
{
    return Speed_;
}

const QString& FinishedJob::GetTimeToComplete () const
{
    return TimeToComplete_;
}

void FinishedJob::FeedVariantList (const QVariantList& params)
{
    if (params.size () != 5)
        throw Exceptions::InvalidParameter ("FinishedJob::FeedVariantList(): wrong number of arguments.");
    Local_            = params [0].toString ();
    URL_            = params [1].toString ();
    Size_            = params [2].value<ImpBase::length_t> ();
    Speed_            = params [3].toString ();
    TimeToComplete_    = params [4].toString ();
}

FinishedJob* FinishedJob::FromVariantList (const QVariantList& params)
{
    FinishedJob *result = new FinishedJob;
    result->FeedVariantList (params);

    return result;
}

