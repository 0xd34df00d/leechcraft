CREATE TABLE mrss_thumbnails (
    mrss_thumb_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL, 
    url TEXT, 
    width INTEGER, 
    height INTEGER, 
    time TEXT
);

ALTER TABLE mrss_thumbnails ADD 
  FOREIGN KEY ( mrss_id ) 
    REFERENCES mrss ( mrss_id )
      ON DELETE CASCADE 
      ON UPDATE CASCADE ;
