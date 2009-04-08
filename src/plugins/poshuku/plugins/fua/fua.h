#ifndef FUA_H
#define FUA_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QMap>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/pluginbase.h>

class QStandardItemModel;
class Settings;

class Poshuku_Fua : public QObject
				  , public IInfo
				  , public IPlugin2
				  , public LeechCraft::Poshuku::PluginBase
{
	Q_OBJECT
	Q_INTERFACES (IInfo IPlugin2 LeechCraft::Poshuku::PluginBase)

	boost::shared_ptr<QStandardItemModel> Model_;
	boost::shared_ptr<Settings> Settings_;
	QMap<QString, QString> BrowserToID_;
public:
	void Init ();
	void Release ();
	QString GetName () const;
	QString GetInfo () const;
	QIcon GetIcon () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);

	void Init (LeechCraft::Poshuku::IProxyObject*);
	QByteArray GetPluginClass () const;

	QString OnUserAgentForUrl (const QWebPage*, const QUrl&);
};

#endif

