SELECT History.Title, History.TS, GROUP_CONCAT(Tags.TagLCId, ';')
FROM History
	JOIN TagsMapping USING (EntryId)
	JOIN Tags USING (TagId)
GROUP BY History.EntryId;
