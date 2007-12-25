#include <QDateTime>
#include <QtDebug>
#include "connectionmanager.h"
#include "settingsmanager.h"

Q_GLOBAL_STATIC (ConnectionManager, ConnectionManagerInstance);

ConnectionManager* ConnectionManager::Instance ()
{
	return ConnectionManagerInstance ();
}

bool ConnectionManager::CanAddConnection () const
{
	return (Connections_.size () < GetMaxConnections ());
}

void ConnectionManager::AddConnection (PeerConnection *pc)
{
	Connections_ << pc;
}

void ConnectionManager::RemoveConnection (PeerConnection *pc)
{
	Connections_.remove (pc);
}

int ConnectionManager::GetMaxConnections () const
{
	return SettingsManager::Instance ()->GetMaxTotalConnections ();
}

QByteArray ConnectionManager::GetClientID () const
{
	if (ID_.isEmpty ())
	{
		int st = static_cast<int> (QDateTime::currentDateTime ().toTime_t ());
		
		QString s ("-LB0001-");
		ID_ += s.toLatin1 ();
		ID_ += QByteArray::number (st, 10);
		ID_ += QByteArray (20 - ID_.size (), '-');
	}
	return ID_;
}

