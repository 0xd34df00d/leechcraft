CREATE TABLE images (
	package_id INTEGER REFERENCES packages,
	url TEXT NOT NULL,
	type CHAR(1) NOT NULL
);