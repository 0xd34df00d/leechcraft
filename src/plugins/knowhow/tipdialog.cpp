/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "tipdialog.h"
#include <QDomDocument>
#include <util/util.h>
#include <util/resourceloader.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace KnowHow
{
	TipDialog::TipDialog (QWidget *parent)
	: QDialog (parent)
	{
		Util::ResourceLoader rl ("knowhow/tips/");
		rl.AddGlobalPrefix ();
		rl.AddLocalPrefix ();

		const QString& pre = "tips_";
		QStringList vars;
		vars << (pre + Util::GetLocaleName () + ".xml")
			<< (pre + Util::GetLocaleName ().left (2) + ".xml")
			<< (pre + "en.xml");

		const QString& path = rl.GetPath (vars);
		if (path.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to load tips for"
					<< vars;
			deleteLater ();
			return;
		}

		QFile file (path);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			deleteLater ();
			return;
		}

		Doc_.reset (new QDomDocument);
		Doc_->setContent (&file);

		const int idx = XmlSettingsManager::Instance ()
				.Property ("StdTipIndex", -1).toInt () + 1;
		const QString& tip = GetTipByID (idx);
		if (tip.isEmpty ())
		{
			if (idx == 0)
				qWarning () << Q_FUNC_INFO
						<< "empty tip right for the first one!";
			return;
		}

		Ui_.setupUi (this);
		Ui_.TipEdit_->setText (tip);
		setAttribute (Qt::WA_DeleteOnClose, true);
		show ();
	}

	QString TipDialog::GetTipByID (int idx)
	{
		const QString& numIdx = QString::number (idx);

		QDomElement elem = Doc_->firstChildElement ().firstChildElement ("tip");
		while (!elem.isNull ())
		{
			if (elem.attribute ("id") == numIdx)
				break;

			elem = elem.nextSiblingElement ("tip");
		}

		if (elem.isNull ())
			return QString ();

		return elem.text ().trimmed ();
	}
}
}
