#include "externalproxy.h"
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			ExternalProxy::ExternalProxy (QObject *parent)
			: QObject (parent)
			{
			}

			void ExternalProxy::AddSearchProvider (const QString& url)
			{
				LeechCraft::DownloadEntity e;
				e.Entity_ = url.toUtf8 ();
				e.Mime_ = "application/opensearchdescription+xml";
				e.Location_ = url;
				e.Parameters_ = LeechCraft::FromUserInitiated;
				emit gotEntity (e);
			}
			
		};
	};
};

