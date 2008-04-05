#ifndef PROXY_H
#define PROXY_H
#include <QObject>
#include <QStringList>
#include <QTime>
#include "../mainwindow.h"

class TcpSocket;

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

    Main::MainWindow *Window_;

    static Proxy *Instance_;
    QStringList Strings_;
    friend class Main::MainWindow;
public:
    static Proxy *Instance ();
    void SetStrings (const QStringList&);
    TcpSocket* MakeSocket () const;
    QString GetApplicationName () const;
    QString GetOrganizationName () const;
    QString MakePrettySize (qint64) const;
    QTime MakeTimeFromLong (ulong) const;
private:
    void SetMainWindow (Main::MainWindow*);
};

#endif

