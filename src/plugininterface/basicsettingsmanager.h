#ifndef BASICSETTINGSMANAGER_H
#define BASICSETTINGSMANAGER_H
#include <QObject>
#include <QMap>
#include <QByteArray>
#include <interfaces/interfaces.h>

class QSettings;

class BasicSettingsManager : public QObject
{
    Q_OBJECT

	QMap<QByteArray, QPair<QObject*, QByteArray> > Properties2Object_;
	bool Initializing_;
public:
    BasicSettingsManager ();
    ~BasicSettingsManager ();
	void Init ();
    void Release ();
	void RegisterObject (const QByteArray&, QObject*, const QByteArray&);
	QVariant Property (const QString&, const QVariant&);
protected:
    virtual bool event (QEvent*);
    virtual QSettings* BeginSettings () = 0;
    virtual void EndSettings (QSettings*) = 0;
};

#endif

