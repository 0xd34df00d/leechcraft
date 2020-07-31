/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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
#include <util/xpc/util.h>
#include <util/sll/unreachable.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/ientityhandler.h>
#include "shooterdialog.h"
#include "util.h"

namespace LC
{
namespace Auscrie
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("auscrie");

		Dialog_ = new ShooterDialog (proxy);

		ShotAction_ = new QAction (GetIcon (),
				tr ("Make a screenshot"),
				this);
		connect (ShotAction_,
				&QAction::triggered,
				this,
				[this] { MakeScreenshot (0); });
		connect (Dialog_,
				&ShooterDialog::screenshotRequested,
				this,
				[this] { MakeScreenshot (Dialog_->GetTimeout () * 1000); },
				Qt::QueuedConnection);
		connect (Dialog_,
				&QDialog::accepted,
				this,
				&Plugin::PerformAction);
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
		return tr ("Simple auto screenshotter.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;

		if (place == ActionsEmbedPlace::CommonContextMenu)
			result << ShotAction_;

		return result;
	}

	void Plugin::PerformAction ()
	{
		const auto& pm = Dialog_->GetScreenshot ();
		if (pm.isNull ())
			return;

		auto mw = Proxy_->GetRootWindowsManager ()->GetPreferredWindow ();
		const int quality = Dialog_->GetQuality ();

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
			break;
		}
		case ShooterDialog::Action::Upload:
		{
			const auto& info = Dialog_->GetDFInfo ();
			if (!info.Object_)
			{
				qWarning () << Q_FUNC_INFO
						<< "no object set";
				break;
			}

			auto e = Util::MakeEntity (pm.toImage (),
					{}, {}, "x-leechcraft/data-filter-request");
			e.Additional_ ["Format"] = Dialog_->GetFormat ();
			e.Additional_ ["Quality"] = quality;
			e.Additional_ ["DataFilter"] = info.Variant_;

			auto ieh = qobject_cast<IEntityHandler*> (info.Object_);
			ieh->Handle (e);

			const auto iinfo = qobject_cast<IInfo*> (info.Object_);
			SaveFilterState ({ iinfo->GetUniqueID (), info.Variant_ });

			break;
		}
		}
	}

	void Plugin::MakeScreenshot (int timeout)
	{
		Dialog_->setVisible (!Dialog_->ShouldHide ());

		ShotAction_->setEnabled (false);
		QTimer::singleShot (std::max (timeout, 200),
				this,
				[this]
				{
					ShotAction_->setEnabled (true);

					const auto& pm = GetPixmap ();
					Dialog_->show ();
					Dialog_->SetScreenshot (pm);
				});
	}

	QPixmap Plugin::GetPixmap () const
	{
		auto rootWin = Proxy_->GetRootWindowsManager ()->GetPreferredWindow ();

		// Qt folks have decided that grabbing screens is "insecure", so there is no good substitute for this API
		// now that it's deprecated. The suggestion is to use platform-dependent code.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		switch (Dialog_->GetMode ())
		{
		case ShooterDialog::Mode::LCWindowOverlay:
			return QPixmap::grabWindow (rootWin->winId ());
		case ShooterDialog::Mode::LCWindow:
			return QPixmap::grabWidget (rootWin);
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
#pragma GCC diagnostic pop

		Util::Unreachable ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_auscrie, LC::Auscrie::Plugin);
