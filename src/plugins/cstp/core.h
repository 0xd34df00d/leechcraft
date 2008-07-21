#ifndef CORE_H
#define CORE_H
#include <QObject>

class Core : public QObject
{
	Q_OBJECT
	
	explicit Core ();
public:
	virtual ~Core ();
	static Core& Instance ();
	void Release ();
};

#endif

