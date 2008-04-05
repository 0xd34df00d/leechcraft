#include <QCoreApplication>
#include "../mainwindow.h"
#include "proxy.h"
#include "tcpsocket.h"

Proxy *Proxy::Instance_ = 0;

Proxy::Proxy ()
{
    Strings_ << "bytes" << "KB" << "MB" << "GB";
}

Proxy::~Proxy ()
{
}

/*! @brief Returns a Proxy.
 *
 * Returns an instance of Proxy which is unique through whole
 * application and it's plugins.
 *
 * @return Pointer to the Proxy instance.
 */
Proxy* Proxy::Instance ()
{
    if (!Instance_)
        Instance_ = new Proxy;
    return Instance_;
}

/*! @brief Sets strings for sizes.
 *
 * Used to simplify the translation of user interface. Main app
 * could set the translated strings here. First one is for bytes,
 * seoond - for kilobytes, third - megabytes, fourth - gigabytes.
 *
 * @param[in] str Strings for the sizes.
 * @sa MakePrettySize
 */
void Proxy::SetStrings (const QStringList& str)
{
    Strings_ = str;
}

/*! @brief Makes a TCP socket.
 *
 * Makes and returns a TCP socket, possibly registering it in some
 * internal structures to facilitate, for example, bandwidth consume
 * controlling.
 *
 * @return The created socket.
 * @sa TcpSocket
 */
TcpSocket* Proxy::MakeSocket () const
{
    return new TcpSocket;
}

/*! @brief Returns application name.
 *
 * @return The application name.
 * @sa GetOrganizationName
 */
QString Proxy::GetApplicationName () const
{
    return QCoreApplication::applicationName ();
}

/*! @brief Returns organization name.
 *
 * @return The organization name.
 * @sa GetApplicationName
 */
QString Proxy::GetOrganizationName () const
{
    return QCoreApplication::organizationName ();
}

/*! @brief Makes a formatted size from number.
 *
 * Converts, for example, 1048576 to 1.0 MB.
 *
 * @param[in] sourcesize Size in bytes.
 * @return Formatted string.
 *
 * @sa SetStrings
 * @sa MakeTimeFromLong
 */
QString Proxy::MakePrettySize (qint64 sourcesize) const
{
    QString dString = Strings_ [0];
    long double size = sourcesize;
    if (size >= 1024)
    {
        dString = Strings_ [1];
        size /= 1024;
    }
    if (size >= 1024)
    {
        dString = Strings_ [2];
        size /= 1024;
    }
    if (size >= 1024)
    {
        dString = Strings_ [3];
        size /= 1024;
    }

    return QString::number (size, 'f', 1) + " " + dString;
}

/*! @brief Makes a formatted time from number.
 *
 * Converts, for example 256 to 00:04:16.
 *
 * @param[in] time Time interval in seconds.
 * @return Formatted string.
 *
 * @sa MakePrettySize
 */
QTime Proxy::MakeTimeFromLong (ulong time) const
{
    int h = time / 3600;
    int m = (time - h * 3600) / 60;
    int s = time - h * 3600 - m * 60;
    return QTime (h, m, s);
}

void Proxy::SetMainWindow (Main::MainWindow *w)
{
    Window_ = w;
}

