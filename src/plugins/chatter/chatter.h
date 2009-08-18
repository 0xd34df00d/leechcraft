#ifndef PLUGINS_CHATTER_CHATTER_H
#define PLUGINS_CHATTER_CHATTER_H
#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihaveshortcuts.h>

class fsirc;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Chatter
		{
			class Plugin : public QObject
						 , public IInfo
			{
				Q_OBJECT
				Q_INTERFACES (IInfo)
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

				void SetShortcut (int, const QKeySequence&);
				QMap<int, LeechCraft::ActionInfo> GetActionInfo () const;
				QList<QAction*> GetActions () const;
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			private:
				QList<QAction*> Actions_;
				fsirc *fsIrc;
			};
		};
	};
};

#endif

