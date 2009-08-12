#ifndef PLUGINS_SEEKTHRU_SEEKTHRU_H
#define PLUGINS_SEEKTHRU_SEEKTHRU_H
#include <QObject>
#include <QTranslator>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class SeekThru : public QObject
						   , public IInfo
						   , public IFinder
						   , public IHaveSettings
						   , public IEntityHandler
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IFinder IHaveSettings IEntityHandler)

				std::auto_ptr<QTranslator> Translator_;
				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
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

				QStringList GetCategories () const;
				IFindProxy_ptr GetProxy (const LeechCraft::Request&);

				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;

				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				void Handle (LeechCraft::DownloadEntity);
			private slots:
				void handleError (const QString&);
				void handleWarning (const QString&);
			signals:
				void delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**);
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};

#endif

