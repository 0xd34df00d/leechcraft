SELECT settings_id, update_timeout, 
       num_items, item_age, auto_download_enclosures 
    FROM feeds_settings 
        WHERE feed_id = ?
