/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_CORE_ITAGSMANAGER_H
#define INTERFACES_CORE_ITAGSMANAGER_H
#include <optional>
#include <QDataStream>
#include <QHash>
#include <QMetaType>
#include <QObject>
#include <QStringList>

class QAbstractItemModel;

namespace LC
{
	/** @brief An identifier of a tag in `ITagsManager`.
	 *
	 * The identifier of a tag survives renaming the tag. `Id_` is the form
	 * to persist, and `FromStrings()` restores identifiers equal to the
	 * persisted ones.
	 */
	struct TagId
	{
		QString Id_;

		bool operator== (const TagId&) const = default;

		/** @brief Wraps the persisted forms `ids` into identifiers.
		 *
		 * @sa ToStrings()
		 */
		static QList<TagId> FromStrings (const QStringList& ids)
		{
			QList<TagId> result;
			result.reserve (ids.size ());
			for (const auto& id : ids)
				result << TagId { id };
			return result;
		}

		/** @brief Returns the persisted forms of `ids`.
		 *
		 * @sa FromStrings()
		 */
		static QStringList ToStrings (const QList<TagId>& ids)
		{
			QStringList result;
			result.reserve (ids.size ());
			for (const auto& id : ids)
				result << id.Id_;
			return result;
		}
	};

	inline size_t qHash (const TagId& id, size_t seed = 0) noexcept
	{
		return qHash (id.Id_, seed);
	}

	inline QDataStream& operator<< (QDataStream& out, const TagId& id)
	{
		return out << id.Id_;
	}

	inline QDataStream& operator>> (QDataStream& in, TagId& id)
	{
		return in >> id.Id_;
	}
}

Q_DECLARE_METATYPE (LC::TagId)

/** @brief Tags manager's interface.
 *
 * This interface is for communication with the tags manager.
 *
 * Tag names are trimmed and non-blank,
 * and they don't contain the delimiter (per `Util::GetTagsDelimiter()`)
 * the tags are split by. Tags are referred to by their `LC::TagId`s.
 *
 * Object returned by the `GetQObject()` function emits these signals:
 * - `tagsUpdated(const QStringList& tags)` when the tags are updated.
 */
class Q_DECL_EXPORT ITagsManager
{
public:
	virtual ~ITagsManager () {}

	/** @brief Returns the ID of the given `tag`.
	 *
	 * If there is no such tag, it's added to the tag collection and the
	 * id of the new tag is returned.
	 *
	 * If the tag is invalid, `std::nullopt` is returned.
	 *
	 * @param[in] tag The tag that should be identified.
	 * @return The ID of the tag, or `std::nullopt` if `tag` is blank or
	 * contains the delimiter.
	 *
	 * @sa GetTag()
	 * @sa GetIDs()
	 */
	virtual std::optional<LC::TagId> GetID (const QString& tag) = 0;

	/** @brief Returns the IDs of the given `tags`.
	 *
	 * This convenience function invokes `GetID()` for each tag in `tags`
	 * and returns the list of the corresponding tag IDs. The tags
	 * rejected by `GetID()` are skipped.
	 *
	 * @param[in] tags The tags that should be identified.
	 * @return The IDs of the tags.
	 *
	 * @sa GetID()
	 */
	QList<LC::TagId> GetIDs (const QStringList& tags)
	{
		QList<LC::TagId> result;
		for (const auto& tag : tags)
			if (const auto& id = GetID (tag))
				result << *id;
		return result;
	}

	/** @brief Returns the tag with the given `id`.
	 *
	 * If there is no such tag, `std::nullopt` is returned. A sensible
	 * plugin would delete the given id from the list of assigned tags
	 * for all the items with this id.
	 *
	 * @param[in] id The id of the tag.
	 * @return The tag, or `std::nullopt` if there is no tag with the
	 * given `id`.
	 *
	 * @sa GetID()
	 * @sa GetTags()
	 */
	virtual std::optional<QString> GetTag (const LC::TagId& id) const = 0;

	/** @brief Returns the tags with the given `ids`.
	 *
	 * This convenience function invokes `GetTag()` for each tag ID in
	 * `ids` and returns the list of the corresponding tags. The IDs
	 * without a tag are skipped.
	 *
	 * @param[in] ids The ids of the tags.
	 * @return The tags corresponding to the `ids`.
	 *
	 * @sa GetTag()
	 */
	QStringList GetTags (const QList<LC::TagId>& ids) const
	{
		QStringList result;
		for (const auto& id : ids)
			if (const auto& tag = GetTag (id))
				result << *tag;
		return result;
	}

	/** @brief Returns all tags existing in LeechCraft now.
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

	/** @brief Splits the given string with tags and returns tags IDs.
	 *
	 * This function is essentially a combination of `Split()` and `GetID()`.
	 * First, the given string is split into human-readable tags, and
	 * then for each tag its ID is obtained.
	 *
	 * @param[in] string String with tags.
	 * @return The list of the tags IDs.
	 */
	virtual QList<LC::TagId> SplitToIDs (const QString& string) = 0;

	/** @brief Joins the given tags into one string that's suitable to
	 * display to the user.
	 *
	 * @param[in] tags List of tags.
	 * @return The joined string with tags.
	 */
	virtual QString Join (const QStringList& tags) const = 0;

	/** @brief Joins the given tag IDs into one human-readable string.
	 *
	 * This function is essentially a combination of `GetTag()` and `Join()`.
	 * First, it converts all given `tagIDs` into tag names using `GetTag()`
	 * and then joins them using `Join()`. This function is provided for
	 * convenience.
	 *
	 * @param[in] tagIDs List of tag IDs.
	 * @return The joined string with tags.
	 */
	virtual QString JoinIDs (const QList<LC::TagId>& tagIDs) const = 0;

	/** @brief Returns the completion model for this tags manager.
	 *
	 * The returned completion model can be used in a QCompleter class.
	 * It uses the tags from this tags manager to provide completions as
	 * the user types.
	 *
	 * @return The completion model suitable for usage in a QCompleter.
	 */
	virtual QAbstractItemModel* GetModel () = 0;

	/** @brief Returns the tags manager as a QObject to get access to
	 * all the meta-stuff.
	 *
	 * @return This object as a QObject.
	 */
	virtual QObject* GetQObject () = 0;
};

Q_DECLARE_INTERFACE (ITagsManager, "org.Deviant.LeechCraft.ITagsManager/1.0")

#endif
