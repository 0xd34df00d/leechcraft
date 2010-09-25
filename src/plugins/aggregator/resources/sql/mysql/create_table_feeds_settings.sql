CREATE TABLE feeds_settings (
    settings_id BIGINT PRIMARY KEY, 
    feed_id BIGINT UNIQUE REFERENCES feeds ON DELETE CASCADE, 
    update_timeout INTEGER NOT NULL, 
    num_items INTEGER NOT NULL, 
    item_age INTEGER NOT NULL, 
    auto_download_enclosures TINYINT NOT NULL
) Engine=InnoDB;

