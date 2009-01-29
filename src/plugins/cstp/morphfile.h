#ifndef MORPHFILE_H
#define MORPHFILE_H
#include <QFile>

class MorphFile : public QFile
{
	Q_OBJECT

	bool Gunzip_;
	int Counter_;
public:
	MorphFile (const QString&);
	MorphFile (QObject*);
	MorphFile (const QString&, QObject*);
	virtual ~MorphFile ();

	void Gunzip (bool);

	void AddRef ();
	void Release ();
};

void intrusive_ptr_add_ref (MorphFile*);
void intrusive_ptr_release (MorphFile*);

#endif

