/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "docstatemanager.h"
#include <util/sll/serializejson.h>
#include <util/sll/parsejson.h>
#include <util/sll/either.h>
#include <util/sll/typegetter.h>
#include <util/util.h>
#include <util/sys/paths.h>
#include "common.h"

namespace LC
{
namespace Monocle
{
	namespace
	{
		QString GetFileName (const QString& id)
		{
			return id.at (0) + '/' + id + ".json";
		}
	}

	DocStateManager::DocStateManager (QObject *parent)
	: QObject (parent)
	, DocDir_ (Util::CreateIfNotExists ("monocle/docstate"))
	{
	}

	void DocStateManager::SetState (const QString& id, const State& state)
	{
		const auto& filename = DocDir_.absoluteFilePath (GetFileName (id));
		if (!DocDir_.exists (id.at (0)))
			DocDir_.mkdir (id.at (0));

		QVariantMap stateMap
		{
			{ "page", state.CurrentPage_ },
			{ "scale", state.CurrentScale_ },
			{ "layout", LayoutMode2Name (state.Lay_) }
		};

		auto& scaleModeStr = stateMap ["scaleMode"];
		switch (state.ScaleMode_)
		{
		case ScaleMode::FitWidth:
			scaleModeStr = "fitWidth";
			break;
		case ScaleMode::FitPage:
			scaleModeStr = "fitPage";
			break;
		case ScaleMode::Fixed:
			scaleModeStr = "fixed";
			break;
		}

		Util::SerializeJsonToFile (filename, stateMap);
	}

	auto DocStateManager::GetState (const QString& id) const -> State
	{
		State result = { 0, LayoutMode::OnePage, -1, ScaleMode::FitWidth };
		const auto& filename = DocDir_.absoluteFilePath (GetFileName (id));
		if (!QFile::exists (filename))
			return result;

		QFile file { filename };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "error reading"
					<< filename
					<< file.errorString ();
			return result;
		}

		const auto& map = Util::ParseJson (&file, Q_FUNC_INFO).toMap ();

		auto set = [&map] (auto& field, const QString& name)
		{
			using Type_t = std::decay_t<decltype (field)>;
			const auto& var = map.value (name);
			if (var.canConvert<Type_t> ())
				field = var.value<Type_t> ();
		};

		set (result.CurrentPage_, "page");
		set (result.CurrentScale_, "scale");

		auto setF = [&set] (auto& field, const QString& name, auto cvt)
		{
			std::decay_t<Util::ArgType_t<decltype (cvt), 0>> storedValue;
			set (storedValue, name);
			field = cvt (std::move (storedValue));
		};
		setF (result.Lay_, "layout", &Name2LayoutMode);
		setF (result.ScaleMode_, "scaleMode",
				[] (QString str)
				{
					if (str == "fitWidth")
						return ScaleMode::FitWidth;
					else if (str == "fitPage")
						return ScaleMode::FitPage;
					else
						return ScaleMode::Fixed;
				});

		return result;
	}
}
}
