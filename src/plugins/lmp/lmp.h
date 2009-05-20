#ifndef PLUGINS_LMP_LMP_H
#define PLUGINS_LMP_LMP_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <QAction>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/itoolbarembedder.h>

class QToolBar;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			class LMP : public QObject
					  , public IInfo
					  , public IHaveSettings
					  , public IEntityHandler
					  , public IToolBarEmbedder
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IHaveSettings IEntityHandler IToolBarEmbedder)

				std::auto_ptr<QTranslator> Translator_;
				boost::shared_ptr<Util::XmlSettingsDialog> SettingsDialog_;
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

				boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;

				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				void Handle (LeechCraft::DownloadEntity);

				QList<QAction*> GetActions () const;
			signals:
				void bringToFront ();
			};
		};
	};
};

#endif

