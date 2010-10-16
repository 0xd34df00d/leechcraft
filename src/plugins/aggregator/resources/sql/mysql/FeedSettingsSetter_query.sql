REPLACE INTO feeds_settings 
    (feed_id, settings_id, update_timeout, num_items, item_age, auto_download_enclosures) 
        VALUES (?, ?, ?, ?, ?, ? );

