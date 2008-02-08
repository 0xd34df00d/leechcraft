#ifndef JOBPARAMS_H
#define JOBPARAMS_H
#include <QString>
#include <QVariantList>
#include "impbase.h"

class JobRepresentation;

struct JobParams
{
 bool IsFullName_;
 QString URL_;
 QString LocalName_;
 bool Autostart_, ShouldBeSavedInHistory_;
 ImpBase::length_t Size_;
 long DownloadTime_;

 JobParams ();
 explicit JobParams (const JobRepresentation&);
 QVariantList ToVariantList () const;
 void FeedVariantList (const QVariantList&);
 static JobParams* FromVariantList (const QVariantList&);
};

#endif

