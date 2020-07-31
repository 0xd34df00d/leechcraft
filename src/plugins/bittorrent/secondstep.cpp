/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QFileDialog>
#include <util/util.h>
#include "secondstep.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace BitTorrent
{
	SecondStep::SecondStep (QWidget *parent)
	: QWizardPage (parent)
	{
		setupUi (this);
	}

	QStringList SecondStep::GetPaths () const
	{
		QStringList result;
		for (int i = 0; i < FilesWidget_->topLevelItemCount (); ++i)
			result << FilesWidget_->topLevelItem (i)->text (1);
		return result;
	}

	void SecondStep::on_AddPath__released ()
	{
		QStringList paths = QFileDialog::getOpenFileNames (this,
				tr ("Select one or more paths to add"),
				XmlSettingsManager::Instance ()->
					property ("LastAddDirectory").toString ());
		if (paths.isEmpty ())
			return;

		XmlSettingsManager::Instance ()->setProperty ("LastAddDirectory",
				paths.at (0));

		QStringList files = paths;
		for (int i = 0; i < files.size (); ++i)
		{
			QString path = files.at (i);
			QTreeWidgetItem *item = new QTreeWidgetItem (FilesWidget_);
			item->setText (0,
					LC::Util::MakePrettySize (QFileInfo (path).size ()));
			item->setText (1, path);
		}
	}

	void SecondStep::on_RemoveSelected__released ()
	{
		QList<QTreeWidgetItem*> items = FilesWidget_->selectedItems ();
		qDeleteAll (items);
	}

	void SecondStep::on_Clear__released ()
	{
		FilesWidget_->clear ();
	}

}
}
