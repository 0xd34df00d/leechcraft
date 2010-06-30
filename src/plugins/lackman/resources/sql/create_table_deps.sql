CREATE TABLE deps (
	package_id INTEGER REFERENCES packages,
	name TEXT NOT NULL,
	version TEXT NOT NULL,
	type CHAR(1) NOT NULL
);