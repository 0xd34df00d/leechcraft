#ifndef CRON_H
#define CRON_H
#include <QObject>
#include <QMap>
#include <interfaces/interfaces.h>

class Cron : public QObject
		   , public IInfo
{
	Q_OBJECT
	Q_INTERFACES (IInfo);

	ID_t ID_;
	QMap<QString, QObject*> Providers_;
public:
	void Init ();
	QString GetName () const;
	QString GetInfo () const;
	QString GetStatusbarMessage () const;
	IInfo& SetID (ID_t);
	ID_t GetID () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);
	void Release ();
};

#endif

