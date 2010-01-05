/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "downloadentitywrapper.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace PyLC
		{
			DownloadEntityWrapper::DownloadEntityWrapper (const DownloadEntity& e)
			: W_ (e)
			{
			}
			
			QVariant DownloadEntityWrapper::GetEntity () const
			{
				return W_.Entity_;
			}
			
			QString DownloadEntityWrapper::GetLocation () const
			{
				return W_.Location_;
			}
			
			QString DownloadEntityWrapper::GetMime () const
			{
				return W_.Mime_;
			}
			
			TaskParameters DownloadEntityWrapper::GetParameters () const
			{
				return W_.Parameters_;
			}
			
			QMap<QString, QVariant> DownloadEntityWrapper::GetAdditional () const
			{
				return W_.Additional_;
			}
			
			void DownloadEntityWrapper::SetEntity (const QVariant& e)
			{
				W_.Entity_ = e;
			}
			
			void DownloadEntityWrapper::SetLocation (const QString& location)
			{
				W_.Location_ = location;
			}
			
			void DownloadEntityWrapper::SetMime (const QString& mime)
			{
				W_.Mime_ = mime;
			}
			
			void DownloadEntityWrapper::SetParameters (const TaskParameters& p)
			{
				W_.Parameters_ = p;
			}
			
			void DownloadEntityWrapper::SetAdditional (const QMap<QString, QVariant>& additional)
			{
				W_.Additional_ = additional;
			}
		};
	};
};

