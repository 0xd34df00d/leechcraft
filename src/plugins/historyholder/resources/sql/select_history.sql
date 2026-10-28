SELECT History.Title, History.TS, GROUP_CONCAT(Tags.TagLCId, ';')
FROM History
	LEFT JOIN TagsMapping USING (EntryId)
	LEFT JOIN Tags USING (TagId)
GROUP BY History.EntryId;
