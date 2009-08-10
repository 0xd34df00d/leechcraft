#include "filescheme.h"
#include <typeinfo>
#include <boost/bind.hpp>
#include <plugininterface/util.h>
#include "schemereply.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace FileScheme
				{
					void FileScheme::Init (ICoreProxy_ptr proxy)
					{
						Translator_.reset (Util::InstallTranslator ("poshuku_filescheme"));

						proxy->RegisterHook (HookSignature<HIDNetworkAccessManagerCreateRequest>::Signature_t (
									boost::bind (&FileScheme::CreateRequest,
										this,
										_1,
										_2,
										_3,
										_4)));
					}

					void FileScheme::Release ()
					{
					}

					QString FileScheme::GetName () const
					{
						return "Poshuku FileScheme";
					}

					QString FileScheme::GetInfo () const
					{
						return tr ("Provides support for file:// scheme.");
					}

					QIcon FileScheme::GetIcon () const
					{
						return QIcon (":/plugins/poshuku/plugins/filescheme/resources/images/poshuku_filescheme.svg");
					}

					QStringList FileScheme::Provides () const
					{
						return QStringList ("file://");
					}

					QStringList FileScheme::Needs () const
					{
						return QStringList ();
					}

					QStringList FileScheme::Uses () const
					{
						return QStringList ();
					}

					void FileScheme::SetProvider (QObject*, const QString&)
					{
					}

					QByteArray FileScheme::GetPluginClass () const
					{
						return QByteArray (typeid (PluginBase).name ());
					}

					void FileScheme::Init (IProxyObject*)
					{
					}

					QNetworkReply* FileScheme::CreateRequest (IHookProxy_ptr proxy,
							QNetworkAccessManager::Operation *op,
							const QNetworkRequest *req,
							QIODevice **)
					{
						if (*op != QNetworkAccessManager::GetOperation)
							return 0;

						QString path = req->url ().toLocalFile ();

						if (!QFileInfo (path).isDir ())
							return 0;

						proxy->CancelDefault ();
						return new SchemeReply (*req, this);
					}
				};
			};
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_filescheme,
		LeechCraft::Plugins::Poshuku::Plugins::FileScheme::FileScheme);

