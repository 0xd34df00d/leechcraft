#ifndef IENTITYHANDLER_H
#define IENTITYHANDLER_H
#include <QByteArray>
#include <QtPlugin>
#include "structures.h"

class IEntityHandler
{
public:
	virtual bool CouldHandle (const QByteArray&,
			LeechCraft::TaskParameters) const = 0;
	virtual void Handle (const QByteArray&,
			LeechCraft::TaskParameters) = 0;

	virtual ~IEntityHandler () {}
};

Q_DECLARE_INTERFACE (IEntityHandler, "org.Deviant.LeechCraft.IEntityHandler/1.0");

#endif

