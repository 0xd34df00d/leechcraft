#include <exceptions/invalidparameter.h>
#include <QtDebug>
#include "jobparams.h"
#include "jobrepresentation.h"

JobParams::JobParams ()
{
}

JobParams::JobParams (const JobRepresentation& jr)
: IsFullName_ (false)
, URL_ (jr.URL_)
, LocalName_ (jr.LocalName_)
, Autostart_ (false)
, ShouldBeSavedInHistory_ (true)
, Size_ (jr.Size_)
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
	Size_ = params [4].value<ImpBase::length_t> ();
	IsFullName_ = true;
}

JobParams* JobParams::FromVariantList (const QVariantList& params)
{
	JobParams *jp = new JobParams;
	jp->FeedVariantList (params);
	return jp;
}

