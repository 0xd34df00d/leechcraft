#ifndef BASICSETTINGSMANAGER_H
#define BASICSETTINGSMANAGER_H
#include <QObject>
#include <interfaces/interfaces.h>

class QSettings;

class BasicSettingsManager : public QObject
{
    Q_OBJECT
public:
    BasicSettingsManager ();
    ~BasicSettingsManager ();
	void Init ();
    void Release ();
protected:
    virtual bool event (QEvent*);
    virtual QSettings* BeginSettings () = 0;
    virtual void EndSettings (QSettings*) = 0;
};

#endif

