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
		QTreeView* GetMainView () const;
		QModelIndex MapToSource (const QModelIndex&) const;

#define LC_DEFINE_REGISTER(a) void RegisterHook (LeechCraft::HookSignature<LeechCraft::a>::Signature_t);
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
	LC_EXPANDER ((HIDDownloadFinishedNotification));
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_DEFINE_REGISTER
	};
};

#endif

