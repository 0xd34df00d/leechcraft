#include <exceptions/invalidparameter.h>
#include <QtDebug>
#include "jobparams.h"
#include "jobrepresentation.h"

JobParams::JobParams ()
: Autostart_ (false)
, ShouldBeSavedInHistory_ (true)
{
}

JobParams::JobParams (const JobRepresentation& jr)
: URL_ (jr.URL_)
, LocalName_ (jr.LocalName_)
, Autostart_ (false)
, ShouldBeSavedInHistory_ (true)
, StartPosition_ (0)
, EndPosition_ (0)
{
}

QVariantList JobParams::ToVariantList () const
{
    QVariantList result;
    result << URL_
        << LocalName_
        << Autostart_
        << ShouldBeSavedInHistory_
        << StartPosition_
        << EndPosition_;
    return result;
}

void JobParams::FeedVariantList (const QVariantList& params)
{
    if (params.size () < 6)
        throw Exceptions::InvalidParameter ("JobParams::FromVariantList(): wrong number of arguments.");
    URL_ = params [0].toString ();
    LocalName_ = params [1].toString ();
    Autostart_ = params [2].toBool ();
    ShouldBeSavedInHistory_ = params [3].toBool ();
    StartPosition_ = params [4].value<quint64> ();
    EndPosition_ = params [5].value<quint64> ();
}

JobParams* JobParams::FromVariantList (const QVariantList& params)
{
    JobParams *jp = new JobParams;
    jp->FeedVariantList (params);
    return jp;
}

