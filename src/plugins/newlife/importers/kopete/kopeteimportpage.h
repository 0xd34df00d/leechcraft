/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "common/imimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	class KopeteImportPage : public Common::IMImportPage
	{
		Q_OBJECT
	public:
		KopeteImportPage (const ICoreProxy_ptr&, QWidget* = nullptr);
	protected:
		void FindAccounts ();
		void SendImportAcc (QStandardItem*);
		void SendImportHist (QStandardItem*);
	private:
		void ScanProto (const QString& path, const QString& proto);
	};
}
}
}
