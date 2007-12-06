#include <QtDebug>
#include <exceptions/invalidparameter.h>
#include "finishedjob.h"
#include "jobparams.h"
#include "jobrepresentation.h"

FinishedJob::FinishedJob ()
{
}

FinishedJob::FinishedJob (const JobRepresentation& jr, QObject *parent)
: QObject (parent)
, URL_ (jr.URL_)
, Local_ (jr.LocalName_)
, Size_ (jr.Size_)
{
}

FinishedJob::~FinishedJob ()
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

void FinishedJob::FeedVariantList (const QVariantList& params)
{
	if (params.size () != 3)
		throw Exceptions::InvalidParameter ("FinishedJob::FeedVariantList(): wrong number of arguments.");
	Local_	= params [0].toString ();
	URL_	= params [1].toString ();
	Size_	= params [2].value<ImpBase::length_t> ();
}

FinishedJob* FinishedJob::FromVariantList (const QVariantList& params)
{
	FinishedJob *result = new FinishedJob;
	result->FeedVariantList (params);

	return result;
}

