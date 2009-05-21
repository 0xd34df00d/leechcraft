#include "cleanweb.h"
#include <typeinfo>
#include <boost/bind.hpp>
#include <QIcon>
#include <QtDebug>
#include "core.h"

using namespace LeechCraft;
using namespace LeechCraft::Plugins::CleanWeb;

void CleanWeb::Init (ICoreProxy_ptr proxy)
{
	proxy->RegisterHook (HookSignature<HIDNetworkAccessManagerCreateRequest>::Signature_t (
				boost::bind (&Core::Hook,
					&Core::Instance (),
					_1,
					_2,
					_3,
					_4)));
}

void CleanWeb::Release ()
{
}

QString CleanWeb::GetName () const
{
	return "CleanWeb";
}

QString CleanWeb::GetInfo () const
{
	return tr ("Blocks unwanted ads and other stuff frequently seen on the internets.");
}

QIcon CleanWeb::GetIcon () const
{
	return QIcon ();
}

QStringList CleanWeb::Provides () const
{
	return QStringList ();
}

QStringList CleanWeb::Needs () const
{
	return QStringList ();
}

QStringList CleanWeb::Uses () const
{
	return QStringList ();
}

void CleanWeb::SetProvider (QObject*, const QString&)
{
}

Q_EXPORT_PLUGIN2 (leechcraft_cleanweb, CleanWeb);

