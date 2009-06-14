#ifndef PLUGINS_DEADLYRICS_DEADLYRICS_H
#define PLUGINS_DEADLYRICS_DEADLYRICS_H
#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			class DeadLyRicS : public QObject
							 , public IInfo
							 , public IFinder
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IFinder)
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
			};
		};
	};
};

#endif

