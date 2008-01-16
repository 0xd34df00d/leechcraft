#include <QtCore/QtCore>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "core.h"
#include "globals.h"

Core::Core (QObject *parent)
: QObject (parent)
{
	qsrand (QDateTime::currentDateTime ().toTime_t ());
	ReadSettings ();
	TimerID_ = startTimer (1000);
}

void Core::Release ()
{
	writeSettings ();
	killTimer (TimerID_);
}

quint64 Core::AddSingleShot (QDateTime dt)
{
	quint64 id = 0;
	while (UsedIDs_.contains (id = qrand ()));
	qDebug () << id;
	SingleShots_.append (qMakePair (dt, id));
	return id;
}

void Core::timerEvent (QTimerEvent *e)
{
	if (e->timerId () != TimerID_)
		return;

	for (int i = 0; i < SingleShots_.size (); ++i)
		if (SingleShots_.at (i).first <= QDateTime::currentDateTime ())
		{
			emit shot (SingleShots_.at (i).second);
			SingleShots_.removeAt (i--);
		}
}

void Core::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (Globals::Name);
	QVariantList tmpList = settings.value ("UsedIDs").toList ();
	for (int i = 0; i < tmpList.size (); ++i)
		UsedIDs_.append (tmpList.at (i).value<quint64> ());
	settings.endGroup ();
}

void Core::writeSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (Globals::Name);
	QVariantList tmpList;
	for (int i = 0; i < UsedIDs_.size (); ++i)
		tmpList.append (UsedIDs_.at (i));
	settings.setValue ("UsedIDs", tmpList);
	settings.endGroup ();
}

