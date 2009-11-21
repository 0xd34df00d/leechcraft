/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_SUMMARY_OPERATIONALMODEL_H
#define PLUGINS_SUMMARY_OPERATIONALMODEL_H
#include <plugininterface/mergemodel.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			class OperationalModel : public Util::MergeModel
			{
				Q_OBJECT
			public:
				enum Operation
				{
					OpNull,
					OpAnd,
					OpOr
				};
			private:
				Operation Op_;
			public:
				OperationalModel (QObject* = 0);
				void SetOperation (Operation);
			protected:
				bool AcceptsRow (QAbstractItemModel*, int) const;
			};
		};
	};
};

#endif

