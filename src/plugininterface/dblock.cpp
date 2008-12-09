#include "dblock.h"
#include <stdexcept>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtDebug>

LeechCraft::Util::DBLock::DBLock (QSqlDatabase& database)
: Database_ (database)
, Good_ (false)
, Initialized_ (false)
{
}

LeechCraft::Util::DBLock::~DBLock ()
{
	if (!Initialized_)
		return;

	if (Good_ ? !Database_.commit () : !Database_.rollback ())
		DumpError (Database_.lastError ());
}

void LeechCraft::Util::DBLock::Init ()
{
	if (!Database_.transaction ())
	{
		DumpError (Database_.lastError ());
		throw std::runtime_error ("Could not start transaction");
	}
	Initialized_ = true;
}

void LeechCraft::Util::DBLock::Good ()
{
	Good_ = true;
}

void LeechCraft::Util::DBLock::DumpError (const QSqlError& lastError)
{
	qWarning () << lastError.text () << "|"
		<< lastError.databaseText () << "|"
		<< lastError.driverText () << "|"
		<< lastError.type () << "|"
		<< lastError.number ();
}

void LeechCraft::Util::DBLock::DumpError (const QSqlQuery& lastQuery)
{
	qWarning () << lastQuery.lastQuery ();
	DumpError (lastQuery.lastError ());
}

