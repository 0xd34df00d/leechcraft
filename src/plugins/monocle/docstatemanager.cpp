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
#include <util/sll/qtutil.h>
#include <util/sll/typegetter.h>
#include <util/util.h>
#include <util/sys/paths.h>

namespace LC::Monocle
{
	namespace
	{
		QString GetFileName (const QString& id)
		{
			return id.at (0) + '/' + id + ".json";
		}
	}

	DocStateManager::DocStateManager (QObject *parent)
	: QObject { parent }
	, DocDir_ { Util::CreateIfNotExists ("monocle/docstate") }
	{
	}

	void DocStateManager::SetState (const QString& id, const State& state)
	{
		const auto& filename = DocDir_.absoluteFilePath (GetFileName (id));
		if (!DocDir_.exists (id.at (0)))
			DocDir_.mkdir (id.at (0));

		QVariantMap stateMap
		{
			{ "page"_qs, state.CurrentPage_ },
			{ "layout"_qs, LayoutMode2Name (state.Lay_) }
		};

		stateMap ["scaleMode"_qs] = Util::Visit (state.ScaleMode_,
				[] (FitPage) { return "fitPage"_qs; },
				[] (FitWidth) { return "fitWidth"_qs; },
				[&] (FixedScale fixed)
				{
					stateMap ["scale"_qs] = fixed.Scale_;
					return "fixed"_qs;
				});

		Util::SerializeJsonToFile (filename, stateMap);
	}

	auto DocStateManager::GetState (const QString& id) const -> State
	{
		State result = { 0, LayoutMode::OnePage, FitWidth {} };
		const auto& filename = DocDir_.absoluteFilePath (GetFileName (id));
		if (!QFile::exists (filename))
			return result;

		QFile file { filename };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << "error reading"
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
		set (result.CurrentPage_, "page"_qs);

		auto setF = [&set] (auto& field, const QString& name, auto cvt)
		{
			std::decay_t<Util::ArgType_t<decltype (cvt), 0>> storedValue;
			set (storedValue, name);
			field = cvt (std::move (storedValue));
		};
		setF (result.Lay_, "layout"_qs, &Name2LayoutMode);
		setF (result.ScaleMode_, "scaleMode"_qs,
				[&] (const QString& str) -> ScaleMode
				{
					if (str == "fitWidth"_qs)
						return FitWidth {};
					if (str == "fitPage"_qs)
						return FitPage {};

					FixedScale s;
					set (s.Scale_, "scale"_qs);
					return s;
				});

		return result;
	}
}
