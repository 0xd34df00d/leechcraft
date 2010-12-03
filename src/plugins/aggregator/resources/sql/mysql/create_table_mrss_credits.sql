CREATE TABLE mrss_credits (
    mrss_credits_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL, 
    role TEXT, 
    who TEXT
);

ALTER TABLE mrss_credits ADD 
  FOREIGN KEY ( mrss_id ) 
    REFERENCES mrss ( mrss_id )
      ON DELETE CASCADE 
      ON UPDATE CASCADE ;
