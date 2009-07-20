#ifndef PLUGINS_LCFTP_CORE_H
#define PLUGINS_LCFTP_CORE_H
#include <QObject>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class Core : public QObject
			{
				Q_OBJECT

				Core ();
			public:
				static Core& Instance ();
				void Release ();

				QStringList Provides () const;
				bool IsOK (const DownloadEntity&) const;
				int Add (DownloadEntity);
			};
		};
	};
};

#endif

