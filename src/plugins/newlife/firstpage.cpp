/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "firstpage.h"
#include <QVariant>
#include <util/sys/resourceloader.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "abstractimporter.h"

namespace LC
{
namespace NewLife
{
	FirstPage::FirstPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
	}

	int FirstPage::nextId () const
	{
		return StartPages_ [GetImporter ()];
	}

	void FirstPage::SetupImporter (AbstractImporter *ai)
	{
		Util::ResourceLoader loader { "newlife/apps" };
		loader.AddGlobalPrefix ();
		loader.AddLocalPrefix ();

		const auto iconMgr = GetProxyHolder ()->GetIconThemeManager ();

		const auto& names = ai->GetNames ();
		const auto& icons = ai->GetIcons ();
		for (int i = 0; i < std::min (names.size (), icons.size ()); ++i)
		{
			const auto& iconName = icons.at (i);
			auto icon = QIcon { loader.LoadPixmap (iconName) };
			if (icon.isNull ())
				icon = iconMgr->GetIcon (iconName);
			if (icon.isNull ())
				icon = iconMgr->GetPluginIcon ();

			Ui_.SourceApplication_->addItem (icon,
					names.at (i),
					QVariant::fromValue<QObject*> (ai));
		}

		auto pages = ai->GetWizardPages ();
		if (!pages.isEmpty ())
		{
			const auto first = pages.takeFirst ();
			StartPages_ [ai] = wizard ()->addPage (first);
			for (const auto page : pages)
				wizard ()->addPage (page);
		}
	}

	AbstractImporter* FirstPage::GetImporter () const
	{
		int currentIndex = Ui_.SourceApplication_->currentIndex ();
		if (currentIndex == -1)
			return 0;

		auto importerObject = Ui_.SourceApplication_->itemData (currentIndex).value<QObject*> ();
		return static_cast<AbstractImporter*> (importerObject);
	}

	QString FirstPage::GetSelectedName () const
	{
		return Ui_.SourceApplication_->currentText ();
	}
}
}
