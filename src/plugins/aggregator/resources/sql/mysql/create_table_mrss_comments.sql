CREATE TABLE mrss_comments (
    mrss_comment_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL, 
    type TEXT, 
    comment TEXT
);

ALTER TABLE mrss_comments ADD 
  FOREIGN KEY ( mrss_id ) 
    REFERENCES mrss ( mrss_id )
      ON DELETE CASCADE 
      ON UPDATE CASCADE ;
