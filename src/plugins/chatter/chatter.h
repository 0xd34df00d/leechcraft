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
						 , public IToolBarEmbedder
						 , public IEntityHandler
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IHaveSettings IToolBarEmbedder IEntityHandler)

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

				QList<QAction*> GetActions () const;
				boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			private:
				QList<QAction*> Actions_;
				fsirc *fsIrc;
				boost::shared_ptr<Util::XmlSettingsDialog> SettingsDialog_;
			};
		};
	};
};

#endif

