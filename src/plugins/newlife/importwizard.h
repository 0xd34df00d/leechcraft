/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizard>
#include <interfaces/core/icoreproxyfwd.h>
#include "ui_importwizard.h"

namespace LC
{
namespace NewLife
{
	class AbstractImporter;
	class FirstPage;

	class ImportWizard : public QWizard
	{
		Q_OBJECT

		friend class FirstPage;

		QObject *Plugin_;

		Ui::ImportWizard Ui_;
		QList<AbstractImporter*> Importers_;
	public:
		ImportWizard (const ICoreProxy_ptr&, QObject*, QWidget* = 0);

		QString GetSelectedName () const;
		QObject* GetPlugin () const;
	private:
		void SetupImporters ();
	};
}
}
