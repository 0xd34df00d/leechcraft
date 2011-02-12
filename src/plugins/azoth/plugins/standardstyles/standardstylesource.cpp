/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "standardstylesource.h"
#include <QtDebug>
#include <plugininterface/resourceloader.h>

namespace LeechCraft
{
namespace Azoth
{
namespace StandardStyles
{
	StandardStyleSource::StandardStyleSource (QObject *parent)
	: QObject (parent)
	, StylesLoader_ (new Util::ResourceLoader ("azoth/styles/standard/", this))
	{
		StylesLoader_->AddGlobalPrefix ();
		StylesLoader_->AddLocalPrefix ();
	}
	
	QAbstractItemModel* StandardStyleSource::GetOptionsModel() const
	{
		return StylesLoader_->GetSubElemModel ();
	}
	
	QString StandardStyleSource::GetHTMLTemplate (const QString& pack) const
	{
		Util::QIODevice_ptr dev = StylesLoader_->Load (QStringList (pack + "/viewcontents.html"));
		if (!dev->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open source file for"
					<< pack + "/viewcontents.html"
					<< dev->errorString ();
			return QString ();
		}
		
		return dev->readAll ();
	}
	
	bool StandardStyleSource::AppendMessage (QWebFrame*, QObject*, const QString&)
	{
		return false;
	}
}
}
}
