#ifndef MORPHFILE_H
#define MORPHFILE_H
#include <QFile>

class MorphFile : public QFile
{
	Q_OBJECT

	bool Gunzip_;
public:
	MorphFile (const QString&);
	MorphFile (QObject*);
	MorphFile (const QString&, QObject*);
	virtual ~MorphFile ();

	void Gunzip (bool);
};

#endif

