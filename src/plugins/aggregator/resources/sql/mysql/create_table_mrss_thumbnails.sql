CREATE TABLE mrss_thumbnails (
    mrss_thumb_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, 
    url TEXT, 
    width INTEGER, 
    height INTEGER, 
    time TEXT
);

