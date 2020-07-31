/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textogroose.h"
#include <QIcon>
#include <QFutureInterface>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <interfaces/iscriptloader.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "apiobject.h"

namespace LC
{
namespace Textogroose
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		const auto& provs = proxy->GetPluginsManager ()->GetAllCastableTo<IScriptLoader*> ();
		for (auto plugin : provs)
		{
			auto ldr = plugin->CreateScriptLoaderInstance ("textogroose");
			ldr->AddGlobalPrefix ();
			ldr->AddLocalPrefix ();
			Loaders_ << ldr;
		}
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Textogroose";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Textogroose";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Script-based song lyrics finder.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QFuture<Plugin::LyricsQueryResult_t> Plugin::RequestLyrics (const Media::LyricsQuery& query)
	{
		const QVariantMap map
		{
			{ "artist", query.Artist_ },
			{ "album", query.Album_ },
			{ "title", query.Title_ }
		};

		QList<IScript_ptr> scripts;
		for (const auto& ldr : Loaders_)
			for (const auto& scriptName : ldr->EnumerateScripts ())
				scripts << ldr->LoadScript (scriptName);

		QFutureInterface<Plugin::LyricsQueryResult_t> promise;
		promise.reportStarted ();
		promise.setExpectedResultCount (scripts.size ());

		for (const auto& script : scripts)
		{
			auto apiObject = new ApiObject (query, script);

			script->AddQObject (apiObject, "API");
			script->InvokeMethod ("searchLyrics", { map });

			connect (apiObject,
					&ApiObject::finished,
					this,
					[promise] (ApiObject *obj, const Media::LyricsResults& lyrics) mutable
					{
						obj->deleteLater ();

						const auto current = promise.resultCount ();
						promise.reportResult (LyricsQueryResult_t { lyrics }, current);
						if (current + 1 == promise.expectedResultCount ())
							promise.reportFinished ();
					});
		}

		return promise.future ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_textogroose, LC::Textogroose::Plugin);
