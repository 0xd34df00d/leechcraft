INSERT INTO installed
(name, version)
SELECT packages.name, packages.version
FROM packages
WHERE packages.package_id = :package_id;