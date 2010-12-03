CREATE TABLE mrss_peerlinks (
    mrss_peerlink_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL, 
    type TEXT, 
    link TEXT
);

ALTER TABLE mrss_peerlinks ADD 
  FOREIGN KEY ( mrss_id ) 
    REFERENCES mrss ( mrss_id )
      ON DELETE CASCADE 
      ON UPDATE CASCADE ;

