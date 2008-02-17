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
{
}

QVariantList JobParams::ToVariantList () const
{
    QVariantList result;
    result << URL_;
    result << LocalName_;
    result << Autostart_;
    result << ShouldBeSavedInHistory_;
    return result;
}

void JobParams::FeedVariantList (const QVariantList& params)
{
    if (params.size () < 4)
        throw Exceptions::InvalidParameter ("JobParams::FromVariantList(): wrong number of arguments.");
    URL_ = params [0].toString ();
    LocalName_ = params [1].toString ();
    Autostart_ = params [2].toBool ();
    ShouldBeSavedInHistory_ = params [3].toBool ();
}

JobParams* JobParams::FromVariantList (const QVariantList& params)
{
    JobParams *jp = new JobParams;
    jp->FeedVariantList (params);
    return jp;
}

