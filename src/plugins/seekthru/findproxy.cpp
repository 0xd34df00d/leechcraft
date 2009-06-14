#include "findproxy.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <plugininterface/mergemodel.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			using LeechCraft::Util::MergeModel;
			
			FindProxy::FindProxy (const LeechCraft::Request& r)
			: R_ (r)
			, MergeModel_ (new MergeModel (QStringList ("1") << "2" << "3"))
			{
			}
			
			FindProxy::~FindProxy ()
			{
				Q_FOREACH (SearchHandler_ptr sh, Handlers_)
					MergeModel_->RemoveModel (sh.get ());
			}
			
			QAbstractItemModel* FindProxy::GetModel ()
			{
				return MergeModel_.get ();
			}
			
			void FindProxy::SetHandlers (const QList<SearchHandler_ptr>& handlers)
			{
				Handlers_ = handlers;
				Q_FOREACH (SearchHandler_ptr sh, Handlers_)
				{
					MergeModel_->AddModel (sh.get ());
					sh->Start (R_);
				}
			}
			
		};
	};
};

