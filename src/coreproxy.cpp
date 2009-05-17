#include "coreproxy.h"
#include "core.h"
#include "mainwindow.h"

using namespace LeechCraft;

CoreProxy::CoreProxy (QObject *parent)
: QObject (parent)
{
}

QNetworkAccessManager* CoreProxy::GetNetworkAccessManager () const
{
	return Core::Instance ().GetNetworkAccessManager ();
}

const IShortcutProxy* CoreProxy::GetShortcutProxy () const
{
	return Core::Instance ().GetShortcutProxy ();
}

QTreeView* CoreProxy::GetMainView () const
{
	return Core::Instance ().GetReallyMainWindow ()->GetMainView ();
}

QModelIndex CoreProxy::MapToSource (const QModelIndex& index) const
{
	return Core::Instance ().MapToSource (index);
}

#define LC_DEFINE_REGISTER(a) \
void CoreProxy::RegisterHook (LeechCraft::HookSignature<LeechCraft::a>::Signature_t functor) \
{ \
	Core::Instance ().RegisterHook (functor); \
}
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
	LC_EXPANDER ((HIDDownloadFinishedNotification));
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_DEFINE_REGISTER
