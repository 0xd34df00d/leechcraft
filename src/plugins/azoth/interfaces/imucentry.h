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

#ifndef PLUGINS_AZOTH_INTERFACES_IMUCENTRY_H
#define PLUGINS_AZOTH_INTERFACES_IMUCENTRY_H
#include <QFlags>
#include <QMetaType>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				class ICLEntry;

				struct ParticipantInfo
				{
				};

				/** @brief Represents a single MUC entry in the CL.
				 *
				 * This class extends ICLEntry by providing methods and
				 * data specific to MUCs. A well-written plugin should
				 * implement this interface along with ICLEntry for MUC
				 * entries.
				 */
				class IMUCEntry
				{
				public:
					virtual ~IMUCEntry () {}

					enum MUCFeature
					{
						/** This room has a configuration dialog and can
						 * be configured.
						 */
						MUCFCanBeConfigured
					};

					Q_DECLARE_FLAGS (MUCFeatures, MUCFeature);

					/** @brief The list of features of this MUC.
					 *
					 * Returns the list of features supported by this
					 * MUC.
					 */
					virtual MUCFeatures GetMUCFeatures () const = 0;

					/** @brief The list of participants of this MUC.
					 *
					 * If the protocol plugin chooses to return info
					 * about participants via the IAccount interface,
					 * the ICLEntry objects returned from this function
					 * and from IAccount should be the same for the same
					 * participants.
					 *
					 * @return The list of participants of this MUC.
					 */
					virtual QList<ICLEntry*> GetParticipants () = 0;
				};

				Q_DECLARE_OPERATORS_FOR_FLAGS (IMUCEntry::MUCFeatures);
			}
		}
	}
}

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Azoth::Plugins::IMUCEntry,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IMUCEntry/1.0");

#endif
