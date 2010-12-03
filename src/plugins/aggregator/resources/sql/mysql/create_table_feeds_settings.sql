CREATE TABLE feeds_settings (
    settings_id BIGINT PRIMARY KEY, 
    feed_id BIGINT UNIQUE, 
    update_timeout INTEGER NOT NULL, 
    num_items INTEGER NOT NULL, 
    item_age INTEGER NOT NULL, 
    auto_download_enclosures TINYINT NOT NULL
) Engine=InnoDB;

ALTER TABLE feeds_settings ADD 
  FOREIGN KEY ( feed_id ) 
    REFERENCES feeds ( feed_id )
      ON DELETE CASCADE 
      ON UPDATE CASCADE ;

