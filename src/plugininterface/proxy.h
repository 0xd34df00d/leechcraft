#ifndef PROXY_H
#define PROXY_H
#include <QObject>
#include <QStringList>
#include <QTime>
#include "config.h"

/*! @brief Provides some common features.
 *
 * Feature versions of Proxy class may include some sort of
 * communications with MainWindow class as it was before removing of
 * LogShower in main LeechCraft application.
 *
 */
class Proxy : public QObject
{
    Q_OBJECT

    Proxy ();
    ~Proxy ();

    static Proxy *Instance_;
    QStringList Strings_;
public:
    LEECHCRAFT_API static Proxy *Instance ();
    LEECHCRAFT_API void SetStrings (const QStringList&);
    LEECHCRAFT_API QString GetApplicationName () const;
    LEECHCRAFT_API QString GetOrganizationName () const;
    LEECHCRAFT_API QString MakePrettySize (qint64) const;
    LEECHCRAFT_API QString MakeTimeFromLong (ulong) const;
};

#endif

