#ifndef PLUGINS_CLEANWEB_CLEANWEB_H
#define PLUGINS_CLEANWEB_CLEANWEB_H
#include <QObject>
#include <QMap>
#include <interfaces/iinfo.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CleanWeb
		{
			class CleanWeb : public QObject
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
			};
		};
	};
};

#endif

