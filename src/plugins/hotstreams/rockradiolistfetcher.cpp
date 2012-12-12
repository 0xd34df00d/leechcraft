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

#include "rockradiolistfetcher.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <QNetworkRequest>
#include <QtDebug>

namespace LeechCraft
{
namespace HotStreams
{
	RockRadioListFetcher::RockRadioListFetcher (QStandardItem *item, QNetworkAccessManager *nam, QObject *parent)
	: StreamListFetcherBase (item, nam, parent)
	{
		Request (QNetworkRequest (QUrl ("http://listen.rockradio.com/public3/")));
	}

	QList<StreamListFetcherBase::StreamInfo> RockRadioListFetcher::Parse (const QByteArray& data)
	{
		QList<StreamInfo> result;

		boost::property_tree::ptree pt;
		std::istringstream istr (data.constData ());
		boost::property_tree::read_json (istr, pt);

		for (const auto& v : pt.get_child (""))
		{
			const QByteArray key (v.second.get<std::string> ("key").c_str ());
			StreamInfo info =
			{
				QString::fromUtf8 (v.second.get<std::string> ("name").c_str ()),
				QString::fromUtf8 (v.second.get<std::string> ("description").c_str ()),
				QStringList (),
				QUrl (v.second.get<std::string> ("playlist").c_str ()),
				QUrl ("http://www.rockradio.com/images/channels/" + key + ".jpg"),
				QString (),
				"pls"
			};

			result << info;
		}

		return result;
	}
}
}
