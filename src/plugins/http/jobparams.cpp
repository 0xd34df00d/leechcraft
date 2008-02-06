#include <exceptions/invalidparameter.h>
#include <QtDebug>
#include "jobparams.h"
#include "jobrepresentation.h"

JobParams::JobParams ()
: DownloadTime_ (0)
, Size_ (0)
, Autostart_ (false)
, ShouldBeSavedInHistory_ (true)
{
}

JobParams::JobParams (const JobRepresentation& jr)
: IsFullName_ (false)
, URL_ (jr.URL_)
, LocalName_ (jr.LocalName_)
, Autostart_ (false)
, ShouldBeSavedInHistory_ (true)
, Size_ (jr.Size_)
, DownloadTime_ (jr.DownloadTime_) 
{
}

QVariantList JobParams::ToVariantList () const
{
   QVariantList result;
   result << URL_;
   result << LocalName_;
   result << Autostart_;
   result << ShouldBeSavedInHistory_;
   result << Size_;
   result << static_cast<qulonglong> (DownloadTime_);
   return result;
}

void JobParams::FeedVariantList (const QVariantList& params)
{
   if (params.size () < 5)
      throw Exceptions::InvalidParameter ("JobParams::FromVariantList(): wrong number of arguments.");
   URL_ = params [0].toString ();
   LocalName_ = params [1].toString ();
   Autostart_ = params [2].toBool ();
   ShouldBeSavedInHistory_ = params [3].toBool ();
   Size_ = params [4].value<ImpBase::length_t> ();
   DownloadTime_ = params [5].value<long> ();
   IsFullName_ = true;
}

JobParams* JobParams::FromVariantList (const QVariantList& params)
{
   JobParams *jp = new JobParams;
   jp->FeedVariantList (params);
   return jp;
}

