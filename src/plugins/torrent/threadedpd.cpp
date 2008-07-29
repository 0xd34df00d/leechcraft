#include "threadedpd.h"
#include <QMutexLocker>
#include <QtDebug>

ThreadedPD::ThreadedPD (QWidget *parent, Qt::WindowFlags f)
: QProgressDialog (parent, f)
{
	connect (this, SIGNAL (scheduleIncrement ()),
			this, SLOT (increment ()),
			Qt::QueuedConnection);
}

ThreadedPD::ThreadedPD (const QString& label,
		const QString& cancel,
		int minimum, int maximum,
		QWidget *parent,
		Qt::WindowFlags f)
: QProgressDialog (label, cancel, minimum, maximum, parent, f)
{
	connect (this, SIGNAL (scheduleIncrement ()),
			this, SLOT (increment ()),
			Qt::QueuedConnection);
}

void ThreadedPD::Increment ()
{
	emit scheduleIncrement ();
}

void ThreadedPD::increment ()
{
	setValue (value () + 1);
}

