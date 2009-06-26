#ifndef PLUGINS_POSHUKU_EXTERNALPROXY_H
#define PLUGINS_POSHUKU_EXTERNALPROXY_H
#include <QObject>

namespace LeechCraft
{
	struct DownloadEntity;

	namespace Plugins
	{
		namespace Poshuku
		{
			class ExternalProxy : public QObject
			{
				Q_OBJECT
			public:
				ExternalProxy (QObject* = 0);
			public slots:
				void AddSearchProvider (const QString&);
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};

#endif

