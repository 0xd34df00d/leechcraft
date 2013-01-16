/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "docstatemanager.h"
#include <boost/version.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <util/util.h>
#include "common.h"

#if BOOST_VERSION >= 105000
#define SANE_JSON
#endif

#ifndef SANE_JSON
#pragma warning("Boost 1.50 or higher is NOT available, state management will not work.")
#endif

namespace LeechCraft
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

	namespace bp = boost::property_tree;

	void DocStateManager::SetState (const QString& id, const State& state)
	{
#ifdef SANE_JSON
		const auto& filename = DocDir_.absoluteFilePath (GetFileName (id));
		if (!DocDir_.exists (id.at (0)))
			DocDir_.mkdir (id.at (0));

		bp::ptree pt;
		pt.put ("page", state.CurrentPage_);
		pt.put ("scale", state.CurrentScale_);
		pt.put ("layout", state.Lay_ == LayoutMode::OnePage ? "one" : "two");

		bp::write_json (filename.toUtf8 ().constData (), pt);
#endif
	}

	auto DocStateManager::GetState (const QString& id) const -> State
	{
		State result = { 0, LayoutMode::OnePage, -1 };
#ifdef SANE_JSON
		const auto& filename = DocDir_.absoluteFilePath (GetFileName (id));
		if (!QFile::exists (filename))
			return result;

		bp::ptree pt;
		try
		{
			bp::read_json (filename.toUtf8 ().constData (), pt);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error reading"
					<< filename
					<< e.what ();
			return result;
		}

		if (auto page = pt.get_optional<int> ("page"))
			result.CurrentPage_ = *page;
		if (auto scale = pt.get_optional<double> ("scale"))
			result.CurrentScale_ = *scale;
		if (auto layout = pt.get_optional<std::string> ("layout"))
			result.Lay_ = *layout == "one" ? LayoutMode::OnePage : LayoutMode::TwoPages;
#endif
		return result;
	}
}
}
