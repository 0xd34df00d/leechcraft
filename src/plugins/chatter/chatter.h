#ifndef PLUGINS_CHATTER_CHATTER_H
#define PLUGINS_CHATTER_CHATTER_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/itoolbarembedder.h>
#include <interfaces/iembedtab.h>

class fsirc;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Chatter
		{
			class Plugin : public QObject
						 , public IInfo
						 , public IHaveSettings
						 , public IEntityHandler
						 , public IEmbedTab
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IHaveSettings IEntityHandler IEmbedTab)

				std::auto_ptr<QTranslator> Translator_;
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);

				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				void Handle (LeechCraft::DownloadEntity);

				boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;
				QWidget* GetTabContents ();
				QToolBar* GetToolBar () const;
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
				void bringToFront ();
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void changeTooltip (QWidget*, QWidget*);
				void statusBarChanged (QWidget*, const QString&);
				void raiseTab (QWidget*);
			private:
				QList<QAction*> Actions_;
				fsirc *fsIrc;
				boost::shared_ptr<Util::XmlSettingsDialog> SettingsDialog_;
			};
		};
	};
};

#endif

