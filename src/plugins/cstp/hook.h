#ifndef PLUGINS_CSTP_HOOK_H
#define PLUGINS_CSTP_HOOK_H
#include <QString>
#include <QPair>
#include <QList>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			class Hook
			{
			public:
				virtual void Act (int, const QString&) const;
				virtual void Act (const QList<QPair<QString, QString> >&) const;
			};
		};
	};
};

#endif

