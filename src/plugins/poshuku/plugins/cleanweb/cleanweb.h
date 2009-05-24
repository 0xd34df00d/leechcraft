#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_CLEANWEB_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_CLEANWEB_H
#include <QObject>
#include <QMap>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace CleanWeb
				{
					class CleanWeb : public QObject
								   , public IInfo
								   , public IHaveSettings
								   , public IEntityHandler
					{
						Q_OBJECT
						Q_INTERFACES (IInfo IHaveSettings IEntityHandler)

						boost::shared_ptr<Util::XmlSettingsDialog> SettingsDialog_;
					public:
						void Init (ICoreProxy_ptr);
						void Release ();
						QString GetName () const;
						QString GetInfo () const;
						QIcon GetIcon () const;
						QStringList Provides () const;
						QStringList Needs () const;
						QStringList Uses () const;
						void SetProvider (QObject*, const QString&);

						boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;

						bool CouldHandle (const DownloadEntity&) const;
						void Handle (DownloadEntity);
					signals:
						void delegateEntity (const LeechCraft::DownloadEntity&,
								int*, QObject**);
					};
				};
			};
		};
	};
};

#endif

