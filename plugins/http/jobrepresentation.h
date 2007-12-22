#ifndef JOBREPRESENTATION_H
#define JOBREPRESENTATION_H
#include <QString>
#include "httpimp.h"

struct JobRepresentation
{
	unsigned int ID_;
	QString URL_;
	QString LocalName_;
	ImpBase::length_t Size_;
	ImpBase::length_t Downloaded_;
	double Speed_, CurrentSpeed_;
	long AverageTime_, CurrentTime_;

	bool ShouldBeSavedInHistory_;
};

#endif

