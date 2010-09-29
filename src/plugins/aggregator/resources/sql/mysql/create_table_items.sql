CREATE TABLE items (
    item_id BIGINT PRIMARY KEY, 
    channel_id BIGINT NOT NULL REFERENCES channels ON DELETE CASCADE, 
    title TEXT, 
    url TEXT, 
    description TEXT, 
    author TEXT, 
    category TEXT, 
    guid TEXT, 
    pub_date TIMESTAMP, 
    unread TINYINT, 
    num_comments SMALLINT, 
    comments_url TEXT, 
    comments_page_url TEXT, 
    latitude TEXT, 
    longitude TEXT
);

CREATE INDEX idx_items_channel_id ON items (channel_id);

