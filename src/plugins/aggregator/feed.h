#ifndef FEED_H
#define FEED_H
#include <QString>
#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "channel.h"

struct Feed
{
	/** @brief Contains settings for a feed.
	 * 
	 * If a feed overrides default settings like update timeout, number of
	 * stored items or max items age, this structure is used to store these
	 * settings in the storage backend.
	 *
	 * The feed can override only some of the settings, in this case others
	 * have setting-specific default value.
	 */
	struct FeedSettings
	{
		/** @brief Returns a default-constructed feed.
		 *
		 * A default-constructed feed has all the settings set to default
		 * values - so, no settings are overridden.
		 */
		FeedSettings (int = 0, int = 0, int = 0);

		/** @brief Update timeout for the feed.
		 *
		 * How often the feed should be checked for updates. The default
		 * value is 0.
		 */
		int UpdateTimeout_;
		/** @brief Max number of stored items.
		 *
		 * How much items should be stored in the feed's channels. The
		 * default value is 0.
		 */
		int NumItems_;
		/** @brief Max item's age.
		 *
		 * How old could be items in the feed. The default value is 0.
		 */
		int ItemAge_;
	};

	QString URL_;
	QDateTime LastUpdate_;
	channels_container_t Channels_;

	Feed ();
	Feed (const Feed&);
	Feed& operator= (const Feed&);
};

typedef boost::shared_ptr<Feed> Feed_ptr;
typedef std::vector<Feed_ptr> feeds_container_t;
typedef std::vector<QString> feeds_urls_t;

Q_DECLARE_METATYPE (Feed);

bool operator< (const Feed& f1, const Feed& f2);
QDataStream& operator<< (QDataStream&, const Feed&);
QDataStream& operator>> (QDataStream&, Feed&);

#endif

