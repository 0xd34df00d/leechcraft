/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "tpi.h"
#include <QIcon>
#include <QAbstractItemModel>
#include <util/sys/paths.h>
#include <util/gui/util.h>
#include "infomodelmanager.h"
#include "tooltipview.h"

namespace LeechCraft
{
namespace TPI
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		ModelMgr_ = new InfoModelManager (proxy);

		QuarkComponent comp;
		comp.Url_ = QUrl::fromLocalFile (Util::GetSysPath (Util::SysPath::QML, "tpi", "TPIQuark.qml"));
		comp.DynamicProps_ << QPair<QString, QObject*> ("TPI_infoModel", ModelMgr_->GetModel ());
		comp.DynamicProps_ << QPair<QString, QObject*> ("TPI_proxy", this);
		Components_ << comp;
	}

	void Plugin::SecondInit ()
	{
		ModelMgr_->SecondInit ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.TPI";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "TPI";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Task Progress Indicator quark plugin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		return Components_;
	}

	void Plugin::hovered (int x, int y, const QRect& geometry)
	{
		if (!TooltipView_)
			TooltipView_ = new TooltipView (ModelMgr_->GetModel (), Proxy_->GetColorThemeManager ());

		TooltipView_->move (Util::FitRect ({ x, y },
				TooltipView_->size (), geometry, Util::NoOverlap));
		TooltipView_->show ();
		TooltipView_->Hovered ();
	}

	void Plugin::hoverLeft ()
	{
		if (TooltipView_)
			TooltipView_->Unhovered ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_tpi, LeechCraft::TPI::Plugin);
