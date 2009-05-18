#ifndef PLUGINS_LMP_LMP_H
#define PLUGINS_LMP_LMP_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <QAction>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>

class QToolBar;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			class LMP : public QObject
					  , public IInfo
					  , public IEntityHandler
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IEntityHandler)

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

				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				void Handle (LeechCraft::DownloadEntity);
			signals:
				void bringToFront ();
			};
		};
	};
};

#endif

