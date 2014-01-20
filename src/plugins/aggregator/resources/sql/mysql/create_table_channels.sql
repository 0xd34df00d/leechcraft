CREATE TABLE channels (
    channel_id BIGINT PRIMARY KEY,
    feed_id BIGINT NOT NULL,
    url TEXT,
    title TEXT,
    display_title TEXT,
    description TEXT,
    last_build TIMESTAMP,
    tags TEXT,
    language TEXT,
    author TEXT,
    pixmap_url TEXT,
    pixmap BLOB,
    favicon BLOB
) Engine=InnoDB;

ALTER TABLE channels ADD
  FOREIGN KEY ( channel_id )
    REFERENCES feeds ( feed_id )
      ON DELETE CASCADE
      ON UPDATE CASCADE ;
