/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H

#include <QStringList>


namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	struct ServerOptions
	{
		QString NetworkName_;
		QString ServerName_;
		QString ServerEncoding_;
		QString ServerPassword_;
		QStringList ServerNicknames_;
		QString ServerRealName_;
		int ServerPort_;
		bool SSL_;
	};

	struct ChannelOptions
	{
		QString ServerName_;
		QString ChannelName_;
		QString ChannelPassword_;
		QString ChannelNickname_;
	};

	enum ConnectionState
	{
		Connected,
		InProcess,
		NotConnected
	};

	bool operator== (const ChannelOptions&, const ChannelOptions&);
};
};
};


#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H
