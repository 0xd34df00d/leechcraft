/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * * This program is free software: you can redistribute it and/or modify
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

#include "auscrie.h"
#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QBuffer>
#include <QDir>
#include <QFileDialog>
#include <QDesktopWidget>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include "shooterdialog.h"
#include "poster.h"

namespace LeechCraft
{
namespace Auscrie
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("auscrie");

		Dialog_ = new ShooterDialog ();

		ShotAction_ = new QAction (GetIcon (),
				tr ("Make a screenshot"),
				this);
		connect (ShotAction_,
				SIGNAL (triggered ()),
				Dialog_,
				SLOT (show ()));
		connect (Dialog_,
				SIGNAL (accepted ()),
				this,
				SLOT (makeScreenshot ()),
				Qt::QueuedConnection);
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Auscrie";
	}

	QString Plugin::GetName () const
	{
		return "Auscrie";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Simple auto screenshoter.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/resources/images/auscrie.svg");
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;

		if (place == AEPCommonContextMenu)
			result << ShotAction_;

		return result;
	}

	void Plugin::makeScreenshot ()
	{
		Dialog_->setVisible (!Dialog_->ShouldHide ());

		ShotAction_->setEnabled (false);
		const int add = Dialog_->GetTimeout () ? 0 : 200;
		QTimer::singleShot (Dialog_->GetTimeout () * 1000 + add,
				this,
				SLOT (shoot ()));
	}

	void Plugin::shoot ()
	{
		ShotAction_->setEnabled (true);

		qDebug () << Q_FUNC_INFO << Dialog_->isVisible ();

		auto mw = Proxy_->GetMainWindow ();

		const QPixmap& pm = GetPixmap ();
		int quality = Dialog_->GetQuality ();
		switch (Dialog_->GetAction ())
		{
		case ShooterDialog::Action::Save:
			{
				QString path = Proxy_->GetSettingsManager ()->
					Property ("PluginsStorage/Auscrie/SavePath",
							QDir::currentPath () + "01." + Dialog_->GetFormat ())
					.toString ();

				QString filename = QFileDialog::getSaveFileName (mw,
						tr ("Save as"),
						path,
						tr ("%1 files (*.%1);;All files (*.*)")
							.arg (Dialog_->GetFormat ()));

				if (!filename.isEmpty ())
				{
					pm.save (filename,
							qPrintable (Dialog_->GetFormat ()),
							quality);
					Proxy_->GetSettingsManager ()->
						setProperty ("PluginsStorage/Auscrie/SavePath",
								filename);
				}
			}
			break;
		case ShooterDialog::Action::Upload:
			{
				QByteArray bytes;
				QBuffer buf (&bytes);
				buf.open (QIODevice::ReadWrite);
				if (!pm.save (&buf,
							qPrintable (Dialog_->GetFormat ()),
							quality))
					qWarning () << Q_FUNC_INFO
						<< "save failed"
						<< qPrintable (Dialog_->GetFormat ())
						<< quality;
				Post (bytes);
				break;
			}
		}
	}

	QPixmap Plugin::GetPixmap () const
	{
		switch (Dialog_->GetMode ())
		{
		case ShooterDialog::Mode::LCWindowOverlay:
			return QPixmap::grabWindow (Proxy_->GetMainWindow ()->winId ());
		case ShooterDialog::Mode::LCWindow:
			return QPixmap::grabWidget (Proxy_->GetMainWindow ());
		case ShooterDialog::Mode::CurrentScreen:
		{
 			auto desk = qApp->desktop ();
			auto screen = desk->screen (desk->screenNumber (QCursor::pos ()));
			auto geom = desk->screenGeometry (QCursor::pos ());
			return QPixmap::grabWindow (screen->winId (),
					geom.x (), geom.y (), geom.width (), geom.height ());
		}
		case ShooterDialog::Mode::WholeDesktop:
			return QPixmap::grabWindow (qApp->desktop ()->winId ());
		}
	}

	void Plugin::Post (const QByteArray& data)
	{
		Poster *p = new Poster (Dialog_->GetHostingService (),
				data,
				Dialog_->GetFormat (),
				Proxy_->GetNetworkAccessManager ());
		connect (p,
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_auscrie, LeechCraft::Auscrie::Plugin);
