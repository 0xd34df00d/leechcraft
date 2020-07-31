/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "folder.h"
#include <QDataStream>

namespace LC
{
namespace Snails
{
	bool operator== (const Folder& f1, const Folder& f2)
	{
		return f1.Type_ == f2.Type_ && f1.Path_ == f2.Path_;
	}
}
}

QDataStream& operator<< (QDataStream& out, const LC::Snails::Folder& folder)
{
	out << static_cast<quint8> (1)
			<< static_cast<quint32> (folder.Type_)
			<< folder.Path_;
	return out;
}

QDataStream& operator>> (QDataStream& in, LC::Snails::Folder& folder)
{
	quint8 version = 0;
	in >> version;
	if (version != 1)
		return in;

	quint32 type = 0;
	in >> type
			>> folder.Path_;

	folder.Type_ = static_cast<LC::Snails::FolderType> (type);

	return in;
}

