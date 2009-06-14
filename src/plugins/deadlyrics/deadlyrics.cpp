#include "deadlyrics.h"
#include <QIcon>
#include "core.h"
#include "findproxy.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			void DeadLyRicS::Init (ICoreProxy_ptr proxy)
			{
				Core::Instance ().SetNetworkAccessManager (proxy->GetNetworkAccessManager ());
			}
			
			void DeadLyRicS::Release ()
			{
			}
			
			QString DeadLyRicS::GetName () const
			{
				return "DeadLyRicS";
			}
			
			QString DeadLyRicS::GetInfo () const
			{
				return tr ("Lyrics Searcher");
			}
			
			QIcon DeadLyRicS::GetIcon () const
			{
				return QIcon ();
			}
			
			QStringList DeadLyRicS::Provides () const
			{
				return QStringList ("search::lyrics");
			}
			
			QStringList DeadLyRicS::Needs () const
			{
				return QStringList ();
			}
			
			QStringList DeadLyRicS::Uses () const
			{
				return QStringList ();
			}
			
			void DeadLyRicS::SetProvider (QObject*, const QString&)
			{
			}
			
			QStringList DeadLyRicS::GetCategories () const
			{
				return Core::Instance ().GetCategories ();
			}
			
			IFindProxy_ptr DeadLyRicS::GetProxy (const LeechCraft::Request& req)
			{
				return IFindProxy_ptr (new FindProxy (req));
			}
			
			Q_EXPORT_PLUGIN2 (leechcraft_deadlyrics, DeadLyRicS);
		};
	};
};

