#ifndef COREPROXY_H
#define COREPROXY_H
#include <QObject>
#include "interfaces/iinfo.h"
#include "plugininterface/guarded.h"

namespace LeechCraft
{
	class CoreProxy : public QObject
					, public ICoreProxy
	{
		Q_OBJECT
		Q_INTERFACES (ICoreProxy);

		Util::Guarded<QList<int> > UsedIDs_;
	public:
		CoreProxy (QObject* = 0);
		QNetworkAccessManager* GetNetworkAccessManager () const;
		const IShortcutProxy* GetShortcutProxy () const;
		QTreeView* GetMainView () const;
		QModelIndex MapToSource (const QModelIndex&) const;
		Util::BaseSettingsManager* GetSettingsManager () const;
		QMainWindow* GetMainWindow () const;
		QIcon GetIcon (const QString&, const QString& = QString ()) const;
		ITagsManager* GetTagsManager () const;
		QStringList GetSearchCategories () const;
		void OpenSummary (const QString&) const;
		int GetID ();
		void FreeID (int);

#define LC_DEFINE_REGISTER(a) void RegisterHook (LeechCraft::HookSignature<LeechCraft::a>::Signature_t);
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
		LC_EXPANDER ((HIDDownloadFinishedNotification)
				(HIDNetworkAccessManagerCreateRequest));
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_DEFINE_REGISTER
	};
};

#endif

