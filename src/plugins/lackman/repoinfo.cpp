/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "repoinfo.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			RepoInfo::RepoInfo ()
			{
			}

			RepoInfo::RepoInfo (const QUrl& url, const QString& name,
					const QString& shortDescr, const QStringList& components)
			: URL_ (url)
			, Name_ (name)
			, ShortDescr_ (shortDescr)
			, Components_ (components)
			{
			}

			const QUrl& RepoInfo::GetUrl () const
			{
				return URL_;
			}

			void RepoInfo::SetUrl (const QUrl& url)
			{
				URL_ = url;
			}

			const QString& RepoInfo::GetName () const
			{
				return Name_;
			}

			void RepoInfo::SetName (const QString& name)
			{
				Name_ = name;
			}

			const QString& RepoInfo::GetShortDescr () const
			{
				return ShortDescr_;
			}

			void RepoInfo::SetShortDescr (const QString& descr)
			{
				ShortDescr_ = descr;
			}

			const QString& RepoInfo::GetLongDescr () const
			{
				return LongDescr_;
			}

			void RepoInfo::SetLongDescr (const QString& descr)
			{
				LongDescr_ = descr;
			}

			const MaintainerInfoList& RepoInfo::GetMaintainers () const
			{
				return Maintainers_;
			}

			void RepoInfo::SetMaintainers (const MaintainerInfoList& maints)
			{
				Maintainers_ = maints;
			}

			const QStringList& RepoInfo::GetComponents () const
			{
				return Components_;
			}

			void RepoInfo::SetComponents (const QStringList& components)
			{
				Components_ = components;
			}
		}
	}
}
