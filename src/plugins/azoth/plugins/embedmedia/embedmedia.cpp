/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "embedmedia.h"
#include <QDebug>
#include <QFileInfo>
#include <QIcon>
#include <QImageReader>
#include <QLabel>
#include <QMessageBox>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/gui/util.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/xpc/util.h>
#include <util/threads/coro.h>
#include <util/threads/coro/context.h>
#include <util/threads/coro/either.h>
#include <util/threads/coro/networkresult.h>

namespace LC::Azoth::EmbedMedia
{
	void Plugin::Init (ICoreProxy_ptr)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.EmbedMedia";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth EmbedMedia"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Enables displaying media objects right in chat windows.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin" };
	}

	namespace
	{
		bool IsImageLink (const QUrl& url)
		{
			if (url.scheme () != "http" && url.scheme () != "https")
				return false;

			static const QStringList imageExts { "gif"_qs, "png"_qs, "jpg"_qs, "jpeg"_qs, "webp"_qs };

			const auto& ext = QFileInfo { url.path () }.suffix ();
			return imageExts.contains (ext, Qt::CaseInsensitive);
		}

		void HandleImageData (const QByteArray& data, const QUrl& url, QWidget *parent)
		{
			const auto& px = QPixmap::fromImage (QImage::fromData (data));
			if (px.isNull ())
			{
				const auto& e = Util::MakeEntity (url, {}, FromUserInitiated | OnlyHandle);
				GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				return;
			}

			const auto& pos = parent ?
					(parent->geometry ().topLeft () + parent->geometry ().bottomRight ()) / 2 :
					QPoint {};
			Util::ShowPixmapLabel (px, pos)->setWindowTitle (url.toString ());
		}
	}

	void Plugin::hookLinkClicked (IHookProxy_ptr proxy, QObject *chatTabObj, const QUrl& url)
	{
		const auto chatTab = qobject_cast<QWidget*> (chatTabObj);

		if (!IsImageLink (url) || !chatTab)
			return;

		proxy->CancelDefault ();

		[] (QUrl url, QWidget *chatTab) -> Util::ContextTask<>
		{
			co_await Util::AddContextObject { *chatTab };

			const auto reply = GetProxyHolder ()->GetNetworkAccessManager ()->get (QNetworkRequest { url });
			const auto result = co_await *reply;
			const auto data = co_await Util::WithHandler (result.ToEither ("Azoth EmbedMedia"_qs),
					[chatTab] (const QString& error)
					{
						QMessageBox::critical (chatTab,
								"Azoth EmbedMedia"_qs,
								tr ("Unable to fetch the image: %1").arg (error));
					});
			HandleImageData (data, url, chatTab);
		} (url, chatTab);
	}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_embedmedia, LC::Azoth::EmbedMedia::Plugin);
