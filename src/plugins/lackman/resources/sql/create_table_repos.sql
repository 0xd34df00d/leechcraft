CREATE TABLE repos (
	repo_id INTEGER PRIMARY KEY AUTOINCREMENT,
	url TEXT NOT NULL UNIQUE,
	name TEXT NOT NULL UNIQUE,
	description TEXT NOT NULL,
	longdescr TEXT,
	maint_name TEXT,
	maint_email TEXT
);