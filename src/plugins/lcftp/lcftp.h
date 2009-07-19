#ifndef PLUGINS_LCFTP_LCFTP_H
#define PLUGINS_LCFTP_LCFTP_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/imultitabs.h>
#include <interfaces/ientityhandler.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class LCFTP : public QObject
						, public IInfo
						, public IMultiTabs
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IMultiTabs)

				std::auto_ptr<QTranslator> Translator_;
			public:
				void Init (ICoreProxy_ptr);
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);
				QIcon GetIcon () const;
			signals:
				void bringToFront ();
				void addNewTab (const QString&, QWidget*);
				void removeTab (QWidget*);
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void changeTooltip (QWidget*, QWidget*);
				void statusBarChanged (QWidget*, const QString&);
				void raiseTab (QWidget*);
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};

#endif

