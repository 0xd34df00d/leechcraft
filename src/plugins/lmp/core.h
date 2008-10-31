#ifndef CORE_H
#define CORE_H
#include <QObject>

namespace Phonon
{
	class MediaObject;
};

class Core : public QObject
{
	Q_OBJECT

	Core ();
public:
	static Core& Instance ();
	void Release ();
	Phonon::MediaObject* CreateObject (const QString&);
};

#endif

