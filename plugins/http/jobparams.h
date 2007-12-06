#ifndef JOBPARAMS_H
#define JOBPARAMS_H
#include <QString>
#include <QVariantList>

class JobRepresentation;

struct JobParams
{
	bool IsFullName_;
	QString URL_;
	QString LocalName_;
	bool Autostart_, ShouldBeSavedInHistory_;

	JobParams ();
	explicit JobParams (const JobRepresentation&);
	QVariantList ToVariantList () const;
	void FeedVariantList (const QVariantList&);
	static JobParams* FromVariantList (const QVariantList&);
};

#endif

