#ifndef PLUGINS_HISTORYHOLDER_HISTORYHOLDER_H
#define PLUGINS_HISTORYHOLDER_HISTORYHOLDER_H
#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ientityhandler.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace HistoryHolder
		{
			class Plugin : public QObject
						 , public IInfo
						 , public IFinder
						 , public IEntityHandler
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IFinder IEntityHandler)
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

				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				void Handle (LeechCraft::DownloadEntity);
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};

#endif

