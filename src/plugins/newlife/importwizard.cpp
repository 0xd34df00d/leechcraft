/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "importwizard.h"
#include <QtDebug>
#include "importers/akregator/akregatorimporter.h"
#include "importers/firefox/firefoximporter.h"
#include "importers/kopete/kopeteimporter.h"
#include "importers/ktorrent/ktorrentimporter.h"
#include "importers/liferea/lifereaimporter.h"
#include "importers/psiplus/psiplusimporter.h"
#include "importers/vacuum/vacuumimporter.h"
#include "importers/opera/operaimporter.h"
#include "importers/jsonbookmarks/jsonbookmarksimporter.h"

namespace LC
{
namespace NewLife
{
	ImportWizard::ImportWizard (const ICoreProxy_ptr& proxy, QObject *plugin, QWidget *parent)
	: QWizard (parent)
	, Plugin_ (plugin)
	{
		Ui_.setupUi (this);

		using namespace Importers;
		std::apply ([&] (auto... tys) { Importers_ = { new std::remove_pointer_t<decltype (tys)> (proxy, this)... }; },
				std::tuple<
						AkregatorImporter*,
						FirefoxImporter*,
						OperaImporter*,
						JsonBookmarksImporter*,
						KTorrentImporter*,
						LifereaImporter*,
						KopeteImporter*,
						PsiPlusImporter*,
						VacuumImporter*
					> {});

		connect (this,
				&QDialog::accepted,
				this,
				&QObject::deleteLater,
				Qt::QueuedConnection);
		connect (this,
				&QDialog::rejected,
				this,
				&QObject::deleteLater,
				Qt::QueuedConnection);

		SetupImporters ();
	}

	QString ImportWizard::GetSelectedName () const
	{
		return Ui_.FirstPage_->GetSelectedName ();
	}

	QObject* ImportWizard::GetPlugin () const
	{
		return Plugin_;
	}

	void ImportWizard::SetupImporters ()
	{
		for (const auto ai : Importers_)
			Ui_.FirstPage_->SetupImporter (ai);
	}
}
}
