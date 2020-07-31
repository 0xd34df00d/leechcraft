/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "optionsconfigwidget.h"
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QtDebug>
#include "kbctl.h"
#include "rulesstorage.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace KBSwitch
{
	OptionsConfigWidget::OptionsConfigWidget (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Code"), tr ("Description") });

		const auto& opts = XmlSettingsManager::Instance ()
				.property ("EnabledOptions").toStringList ();

		const auto& options = KBCtl::Instance ().GetRulesStorage ()->GetOptions ();
		for (auto groupPos = options.begin (); groupPos != options.end (); ++groupPos)
		{
			const auto& submap = *groupPos;

			auto groupRootItem = new QStandardItem (groupPos.key ());
			auto groupRootDesc = new QStandardItem ((*groupPos) [{}]);
			groupRootItem->setEditable (false);
			groupRootDesc->setEditable (false);

			for (auto sgPos = submap.begin (); sgPos != submap.end (); ++sgPos)
			{
				if (sgPos.key ().isEmpty ())
					continue;

				auto optionItem = new QStandardItem (sgPos.key ());
				optionItem->setEditable (false);
				optionItem->setCheckable (true);

				const auto& fullname = groupPos.key () + ':' + sgPos.key ();
				optionItem->setCheckState (opts.contains (fullname) ?
						Qt::Checked :
						Qt::Unchecked);

				auto optionDesc = new QStandardItem (sgPos.value ());
				optionDesc->setEditable (false);

				groupRootItem->appendRow ({ optionItem, optionDesc });
			}

			Model_->appendRow ({ groupRootItem, groupRootDesc });
		}

		KBCtl::Instance ().SetOptions (opts);

		connect (Model_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (markModified ()));

		auto treeView = new QTreeView ();
		auto layout = new QVBoxLayout ();
		layout->setContentsMargins (0, 0, 0, 0);
		layout->addWidget (treeView);
		setLayout (layout);

		treeView->setModel (Model_);
	}

	void OptionsConfigWidget::accept ()
	{
		if (!Modified_)
			return;

		Modified_ = false;

		auto opts = XmlSettingsManager::Instance ()
				.property ("EnabledOptions").toStringList ();
		opts.sort ();

		QStringList newOpts;
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			auto groupItem = Model_->item (i);

			const auto& groupPrefix = groupItem->text ();

			for (int j = 0; j < groupItem->rowCount (); ++j)
			{
				auto optItem = groupItem->child (j);
				if (optItem->checkState () == Qt::Checked)
					newOpts << groupPrefix + ':' + optItem->text ();
			}
		}
		newOpts.sort ();

		if (opts == newOpts)
			return;

		XmlSettingsManager::Instance ().setProperty ("EnabledOptions", newOpts);
		KBCtl::Instance ().SetOptions (newOpts);
	}

	void OptionsConfigWidget::reject ()
	{
		const auto& opts = XmlSettingsManager::Instance ()
				.property ("EnabledOptions").toStringList ();
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			auto groupItem = Model_->item (i);

			const auto& groupPrefix = groupItem->text ();

			for (int j = 0; j < groupItem->rowCount (); ++j)
			{
				auto optItem = groupItem->child (j);
				const auto& fullname = groupPrefix + ':' + optItem->text ();
				optItem->setCheckState (opts.contains (fullname) ?
						Qt::Checked :
						Qt::Unchecked);
			}
		}

		Modified_ = false;
	}

	void OptionsConfigWidget::markModified ()
	{
		Modified_ = true;
	}
}
}
