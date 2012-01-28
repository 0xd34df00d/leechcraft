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

#ifndef INTERFACES_CORE_ITAGSMANAGER_H
#define INTERFACES_CORE_ITAGSMANAGER_H
#include <QStringList>

class QAbstractItemModel;

/** @brief Tags manager's interface.
 *
 * This interface is for communication with the tags manager.
 *
 * Object returned by the GetObject() function emits these signals:
 * - tagsUpdated(const QStringList& tags) when the tags are updated.
 */
class ITagsManager
{
public:
	typedef QString tag_id;

	virtual ~ITagsManager () {}

	/** @brief Returns the ID of the given tag.
	 *
	 * If there is no such tag, it's added to the tag collection and the
	 * id of the new tag is returned.
	 *
	 * @param[in] tag The tag that should be identified.
	 * @return The ID of the tag.
	 *
	 * @sa GetTag
	 */
	virtual tag_id GetID (const QString& tag) = 0;

	/** @brief Returns the tag with the given id.
	 *
	 * If there is no such tag, a null QString is returned. A sensible
	 * plugin would delete the given id from the list of assigned tags
	 * for all the items with this id.
	 *
	 * @param[in] id The id of the tag.
	 * @return The tag.
	 *
	 * @sa GetID
	 */
	virtual QString GetTag (tag_id id) const = 0;

	/** Returns all tags existing in LeechCraft now.
	 *
	 * @return List of all tags.
	 */
	virtual QStringList GetAllTags () const = 0;

	/** @brief Splits the given string with tags to the list of the tags.
	 *
	 * @param[in] string String with tags.
	 * @return The list of the tags.
	 */
	virtual QStringList Split (const QString& string) const = 0;

	/** @brief Joins the given tags into one string that's suitable to
	 * display to the user.
	 *
	 * @param[in] tags List of tags.
	 * @return The joined string with tags.
	 */
	virtual QString Join (const QStringList& tags) const = 0;

	/** @brief Returns the completion model for this.
	 */
	virtual QAbstractItemModel* GetModel () = 0;

	/** @brief Returns the tags manager as a QObject to get access to
	 * all the meta-stuff.
	 */
	virtual QObject* GetObject () = 0;
};

Q_DECLARE_INTERFACE (ITagsManager, "org.Deviant.LeechCraft.ITagsManager/1.0");

#endif
