CREATE TABLE feeds (
    feed_id BIGINT PRIMARY KEY, 
    url VARCHAR(255) UNIQUE NOT NULL, 
    last_update TIMESTAMP 
) Engine=InnoDB;
