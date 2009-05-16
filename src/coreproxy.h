#ifndef COREPROXY_H
#define COREPROXY_H
#include <QObject>
#include "interfaces/iinfo.h"

namespace LeechCraft
{
	class CoreProxy : public QObject
					, public ICoreProxy
	{
		Q_OBJECT
		Q_INTERFACES (ICoreProxy);
	public:
		CoreProxy (QObject* = 0);
		QNetworkAccessManager* GetNetworkAccessManager () const;
		const IShortcutProxy* GetShortcutProxy () const;
	};
};

#endif

