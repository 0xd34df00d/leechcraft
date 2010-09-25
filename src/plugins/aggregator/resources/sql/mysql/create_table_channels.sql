CREATE TABLE channels (
    channel_id BIGINT PRIMARY KEY, 
    feed_id BIGINT NOT NULL REFERENCES feeds ON DELETE CASCADE, 
    url TEXT, 
    title TEXT, 
    description TEXT, 
    last_build TIMESTAMP, 
    tags TEXT, 
    language TEXT, 
    author TEXT, 
    pixmap_url TEXT, 
    pixmap BLOB, 
    favicon BLOB 
) Engine=InnoDB;
