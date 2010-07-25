SELECT repos.repo_id, components.component
FROM repos, components, locations
WHERE repos.repo_id = components.repo_id
AND components.component_id = locations.component_id
AND locations.package_id = :package_id;