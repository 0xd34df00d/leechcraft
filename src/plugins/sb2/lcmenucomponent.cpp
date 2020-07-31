/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lcmenucomponent.h"
#include <QMenu>
#include <QQuickImageProvider>
#include <util/sys/paths.h>
#include <util/util.h>
#include <interfaces/imwproxy.h>

namespace LC
{
namespace SB2
{
	namespace
	{
		class LCMenuImageProvider : public QQuickImageProvider
		{
		public:
			LCMenuImageProvider ()
			: QQuickImageProvider (Pixmap)
			{
			}

			QPixmap requestPixmap (const QString&, QSize*, const QSize&)
			{
				return QPixmap ("lcicons:/resources/images/leechcraft.svg");
			}
		};

		const QString ImageProviderID = "SB2_LCMenuImage";
	}

	LCMenuComponent::LCMenuComponent (IMWProxy *proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Component_ (std::make_shared<QuarkComponent> ("sb2", "LCMenuComponent.qml"))
	{
		Component_->DynamicProps_.append ({ "SB2_menuComponentProxy", this });
		Component_->StaticProps_.append ({ "SB2_menuComponentLCIcon", "image://" + ImageProviderID + "/icon" });
		Component_->StaticProps_.append ({ "SB2_menuTooltipString", tr ("LeechCraft menu") });
		Component_->ImageProviders_.append ({ ImageProviderID, new LCMenuImageProvider });

		Proxy_->HideMainMenu ();
	}

	QuarkComponent_ptr LCMenuComponent::GetComponent () const
	{
		return Component_;
	}

	void LCMenuComponent::execMenu ()
	{
		Proxy_->GetMainMenu ()->exec (QCursor::pos ());
	}
}
}
