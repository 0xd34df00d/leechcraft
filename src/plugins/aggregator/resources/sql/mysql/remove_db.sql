ALTER TABLE feeds DROP CONSTRAINT feeds_pkey;
ALTER TABLE enclosures DROP CONSTRAINT enclosures_pkey;
ALTER TABLE feeds_settings DROP CONSTRAINT feeds_settings_pkey;
ALTER TABLE mrss DROP CONSTRAINT mrss_pkey;
ALTER TABLE mrss_credits DROP CONSTRAINT mrss_credits_pkey;
ALTER TABLE mrss_thumbnails DROP CONSTRAINT mrss_thumbnails_pkey;
DROP INDEX idx_enclosures_item_parents_hash_item_title_item_url;
DROP INDEX idx_channels_parent_feed_url;
DROP INDEX idx_channels_parent_feed_url_title;
DROP INDEX idx_channels_parent_feed_url_title_url;
DROP INDEX idx_items_parents_hash;
DROP INDEX idx_items_parents_hash_title_url;
DROP INDEX idx_items_parents_hash_unread;
DROP INDEX idx_items_title;
DROP INDEX idx_items_url;
DROP INDEX idx_mrss_item_parents_hash_item_title_item_url;
DROP INDEX idx_mrss_item_title;
DROP INDEX idx_mrss_item_url;
DROP INDEX idx_mrss_comments_parent_url_item_parents_hash_item_title_item_;
DROP INDEX idx_mrss_credits_parent_url_item_parents_hash_item_title_item_u;
DROP INDEX idx_mrss_peerlinks_parent_url_item_parents_hash_item_title_item;
DROP INDEX idx_mrss_scenes_parent_url_item_parents_hash_item_title_item_ur;
DROP INDEX idx_mrss_thumbnails_parent_url_item_parents_hash_item_title_ite;

DROP TABLE 
    channels, enclosures, feeds, 
    feeds_settings, items, mrss, 
    mrss_comments, mrss_credits, 
    mrss_peerlinks, mrss_scenes, 
    mrss_thumbnails;

