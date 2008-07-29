#ifndef THREADEDPD_H
#define THREADEDPD_H
#include <QProgressDialog>

class ThreadedPD : public QProgressDialog
{
	Q_OBJECT
public:
	ThreadedPD (QWidget* = 0, Qt::WindowFlags = 0);
	ThreadedPD (const QString&,
			const QString&,
			int, int,
			QWidget* = 0,
			Qt::WindowFlags = 0);
	void Increment ();
private slots:
	void increment ();
signals:
	void scheduleIncrement ();
};

#endif

