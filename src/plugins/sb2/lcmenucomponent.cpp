/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "lcmenucomponent.h"
#include <QMenu>
#include <QDeclarativeImageProvider>
#include <util/sys/paths.h>
#include <util/util.h>
#include <interfaces/imwproxy.h>

namespace LeechCraft
{
namespace SB2
{
	namespace
	{
		class LCMenuImageProvider : public QDeclarativeImageProvider
		{
		public:
			LCMenuImageProvider ()
			: QDeclarativeImageProvider (Image)
			{
			}

			QImage requestImage (const QString&, QSize*, const QSize&)
			{
				return QImage (":/resources/images/leechcraft.svg");
			}
		};

		const QString ImageProviderID = "SB2_LCMenuImage";
	}

	LCMenuComponent::LCMenuComponent (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
		Component_.Url_ = Util::GetSysPath (Util::SysPath::QML, "sb2", "LCMenuComponent.qml");
		Component_.DynamicProps_ << QPair<QString, QObject*> ("SB2_menuComponentProxy", this);

		Component_.StaticProps_ << QPair<QString, QVariant> ("SB2_menuComponentLCIcon", "image://" + ImageProviderID + "/icon");
		Component_.ImageProviders_ << QPair<QString, QDeclarativeImageProvider*> (ImageProviderID, new LCMenuImageProvider);

		Proxy_->GetMWProxy ()->HideMainMenu ();
	}

	QuarkComponent LCMenuComponent::GetComponent () const
	{
		return Component_;
	}

	void LCMenuComponent::execMenu ()
	{
		Proxy_->GetMWProxy ()->GetMainMenu ()->exec (QCursor::pos ());
	}
}
}
