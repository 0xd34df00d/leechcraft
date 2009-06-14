#ifndef ITEM_H
#define ITEM_H
#include <vector>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMetaType>
#include <boost/shared_ptr.hpp>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			struct ItemShort
			{
				QString Title_;
				QString URL_;
				QStringList Categories_;
				QDateTime PubDate_;
				bool Unread_;
			};

			/** Describes an enclosure associated with an item.
			 */
			struct Enclosure
			{
				/* @brief The URL this enclosure refers to.
				 */
				QString URL_;
				/** @brief MIME type of the enclosure.
				 */
				QString Type_;
				/** @brief Length of the attached enclosure or -1 if unknown.
				 */
				qint64 Length_;
				/** @brief  For the Atom's hreflang attribute.
				 */
				QString Lang_;
			};

			struct Item
			{
				/** The title of the item as showed in the item list.
				 */
			    QString Title_;
				/** Link which should be opened when user activates the item, for
				 * example, by double-clicking on the header or by clicking the
				 * appropriate button.
				 */
			    QString Link_;
				/** Main body of the item, showed in the main Aggregator area. Item
				 * contents go here.
				 */
			    QString Description_;
				/** Author of the item.
				 */
			    QString Author_;
				/** Categories of this item.
				 */
			    QStringList Categories_;
				/** Unique ID of the item, but it may be empty because at least
				 * RSS 2.0 standard makes this field optional.
				 */
			    QString Guid_;
				/** Publication datetime of the item. Should be set to invalid
				 * datetime if it could not be determined from the item
				 * representation in the feed.
				 */
			    QDateTime PubDate_;
				/** Indicates whether this item is unread or not.
				 */
			    bool Unread_;
				/** Number of comments for this item. Should be set to -1 if it could
				 * not be determined from the item representation in the feed.
				 */
				int NumComments_;
				/** Link to the comments RSS. Should be left blank if it could not
				 * be determined from the item representation in the feed.
				 */
				QString CommentsLink_;
				/** Link to the page with comments. Should be left blank if it could
				 * not be determined from the item representation in the feed.
				 */
				QString CommentsPageLink_;
				/** List of enclosures of the item.
				 */
				QList<Enclosure> Enclosures_;

				/** Returns the simplified (short) representation of this item.
				 *
				 * @return The simplified (short) representation.
				 */
				ItemShort ToShort () const;
			};

			typedef boost::shared_ptr<Item> Item_ptr;
			typedef std::vector<Item_ptr> items_container_t;
			typedef std::vector<ItemShort> items_shorts_t;

			struct ItemComparator
			{
				Item_ptr Item_;

				ItemComparator (const Item_ptr&);
				bool operator() (const Item_ptr&);
			};

			bool operator== (const Item&, const Item&);
			QDataStream& operator<< (QDataStream&, const Enclosure&);
			QDataStream& operator>> (QDataStream&, Enclosure&);
			QDataStream& operator<< (QDataStream&, const Item&);
			QDataStream& operator>> (QDataStream&, Item&);
			void Print (const Item&);
			void Diff (const Item&, const Item&);

			bool IsModified (Item_ptr, Item_ptr);
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Aggregator::Item);

#endif

