#ifndef CORE_H
#define CORE_H
#include <QObject>
#include "description.h"

class Core : public QObject
{
	Q_OBJECT

	Core ();
public:
	static Core& Instance ();
	void Add (const QString&);
signals:
	void error (const QString&);
};

#endif

