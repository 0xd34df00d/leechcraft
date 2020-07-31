/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "common/imimportpage.h"
#include "common/xmlimaccount.h"

class QStandardItem;

namespace LC
{
namespace NewLife
{
namespace Importers
{
	class PsiPlusImportPage : public Common::IMImportPage
	{
		std::unique_ptr<Common::XMLIMAccount> XIA_;
	public:
		PsiPlusImportPage (const ICoreProxy_ptr& proxy, QWidget* = nullptr);
	protected:
		void FindAccounts () override;
		void SendImportAcc (QStandardItem*) override;
		void SendImportHist (QStandardItem*) override;
	};
}
}
}
