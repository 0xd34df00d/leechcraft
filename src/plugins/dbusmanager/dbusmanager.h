#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H
#include <interfaces/interfaces.h>

class DBusManager : public QObject
				  , public IInfo
{
	Q_OBJECT

	Q_INTERFACES (IInfo);

	unsigned long ID_;
public:
	void Init ();
	void Release ();
	QString GetName () const;
	QString GetInfo () const;
	IInfo& SetID (unsigned long);
	unsigned long GetID () const;
	QStringList Provides () const;
	QStringList Uses () const;
	QStringList Needs () const;
	void SetProvider (QObject*, const QString&);
	QIcon GetIcon () const;
};

#endif

