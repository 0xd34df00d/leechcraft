-- ChatHistory v1 → v2 migration
-- Converts old schema (history.db) to new Oral-based schema (history2.db).
--
-- Usage:
--   cd ~/.leechcraft/azoth/
--   Edit the Config table below (set your nick), then:
--   sqlite3 history2.db < migrate_history.sql
--
-- Prerequisites:
--   1. Run LeechCraft once with the new ChatHistory plugin to create
--      an empty history2.db with the correct Oral-generated schema.
--   2. Both history.db and history2.db must be in the working directory.
--
-- Known limitations (the old schema simply didn't store this data):
--   * AccountName → uses AccountID as a stand-in (self-corrects on next message write via upsert).
--   * MUC private chats (PrivateChat) are detected via a 'conference' domain heuristic —
--     only reliable for XMPP; IRC private chats will migrate as Chat.
--   * Accounts whose EntryIDs don't follow the <AccountID>_<HumanReadableId> pattern
--     are skipped entirely (e.g. UUID-style accounts from old VK plugin).
--   * PersistentId (XEP-0421) → NULL.
--   * Variant on outgoing non-MUC messages → NULL (old schema stored the *remote*
--     resource, new schema expects our *local* resource, which we don't have).
--   * EscapePolicy → dropped (not in new schema).
--   * STATUS / EVENT / SERVICE message types → skipped.

-- ═══════════════════════════════════════════════════════════
-- Configuration — edit before running
-- ═══════════════════════════════════════════════════════════
-- FallbackNick: used as "our" display name for outgoing messages when the
-- nick cannot be derived from the account ID (e.g. non-XMPP accounts).
CREATE TEMP TABLE Config (Key TEXT PRIMARY KEY, Value TEXT NOT NULL);
INSERT INTO Config VALUES ('FallbackNick', 'Me');

-- Speed up the bulk load — safe since old data is preserved in history.db.
PRAGMA journal_mode = MEMORY;
PRAGMA synchronous = OFF;
PRAGMA foreign_keys = OFF;

ATTACH DATABASE 'history.db' AS old;

BEGIN TRANSACTION;


-- ═══════════════════════════════════════════════════════════
-- 0. Build the entry mapping
-- ═══════════════════════════════════════════════════════════
-- The old azoth_users.EntryID stores GetEntryID() output, which has a
-- protocol-specific prefix: <AccountID>_<HumanReadableId>.
--   XMPP:  Xoox.Gloox.XMPP_our@jid.me_their@jid.org
--   IRC:   Acetamide.IRC.1327008969_irc.freenode.net:6667_#chan@srv
--
-- The new schema wants GetHumanReadableID() — just the part after the prefix.
-- Entries that don't match the <AccountID>_ pattern (e.g. UUID accounts) are
-- dropped — we can't recover the correct HumanReadableId for them.
--
-- This temp table is used by both the Entry and Message INSERTs.

CREATE TEMP TABLE CleanEntry (
    AccountId   INTEGER NOT NULL,
    UserId      INTEGER NOT NULL,
    HumanReadableId TEXT NOT NULL,
    Kind        VARCHAR(1) NOT NULL,
    PRIMARY KEY (AccountId, UserId)
);

INSERT INTO CleanEntry (AccountId, UserId, HumanReadableId, Kind)
SELECT
    h.AccountId,
    u.Id,
    SUBSTR(u.EntryID, LENGTH(a.AccountID) + 2),
    CASE
        WHEN SUM(h.Type = 'MUC') > 0 THEN 'M'
        WHEN SUBSTR(u.EntryID, LENGTH(a.AccountID) + 2) LIKE '%@%conference%' THEN 'P'
        ELSE 'C'
    END
FROM old.azoth_history h
JOIN old.azoth_users u ON u.Id = h.Id
JOIN old.azoth_accounts a ON a.Id = h.AccountId
WHERE h.Type IN ('CHAT', 'MUC')
  AND SUBSTR(u.EntryID, 1, LENGTH(a.AccountID) + 1) = a.AccountID || '_'
GROUP BY h.AccountId, u.Id;


-- ═══════════════════════════════════════════════════════════
-- 1. Accounts
-- ═══════════════════════════════════════════════════════════
-- Only migrate accounts that have at least one recoverable entry.
-- AccountId is QByteArray (BLOB) in the new schema, TEXT in the old.
-- CAST ensures type-correct comparisons at runtime (SQLite treats
-- BLOB and TEXT as distinct types — they never compare equal).

INSERT INTO Account (Id, AccountId, AccountName)
SELECT a.Id, CAST(a.AccountID AS BLOB), a.AccountID
FROM old.azoth_accounts a
WHERE EXISTS (SELECT 1 FROM CleanEntry ce WHERE ce.AccountId = a.Id);


-- ═══════════════════════════════════════════════════════════
-- 2. Entries
-- ═══════════════════════════════════════════════════════════
-- DisplayName stores the entry-level display label:
--   Chat        → contact's visible name from the old entry cache, or the bare JID
--   MUC         → room display name from the cache, or the room JID
--   PrivateChat → nick extracted from the JID (after '/').
--                  The old entry cache is NOT used here because it stored the full
--                  participant JID (room@server/nick), not just the nick.

INSERT INTO Entry (Account, HumanReadableId, Kind, DisplayName)
SELECT
    ce.AccountId,
    ce.HumanReadableId,
    ce.Kind,
    CASE
        WHEN ce.Kind = 'P' THEN
            CASE WHEN INSTR(ce.HumanReadableId, '/') > 0
                 THEN SUBSTR(ce.HumanReadableId, INSTR(ce.HumanReadableId, '/') + 1)
                 ELSE ce.HumanReadableId
            END
        ELSE
            COALESCE(
                (SELECT ec.VisibleName FROM old.azoth_entrycache ec WHERE ec.Id = ce.UserId),
                ce.HumanReadableId)
    END
FROM CleanEntry ce;


-- ═══════════════════════════════════════════════════════════
-- 3. MUC context
-- ═══════════════════════════════════════════════════════════
-- Both MUC rooms and MUC private chats get a MucContext row.
--
-- MucName:
--   MUC rooms ('M'):    HumanReadableId IS the room JID (e.g. room@conference.jabber.org)
--   Private chats ('P'): room JID is the part before '/' in the participant JID

INSERT INTO MucContext (Id, MucName)
SELECT Id,
    CASE Kind
        WHEN 'M' THEN HumanReadableId
        WHEN 'P' THEN
            CASE WHEN INSTR(HumanReadableId, '/') > 0
                 THEN SUBSTR(HumanReadableId, 1, INSTR(HumanReadableId, '/') - 1)
                 ELSE HumanReadableId
            END
    END
FROM Entry
WHERE Kind IN ('M', 'P');


-- ═══════════════════════════════════════════════════════════
-- 4. Entry→Id mapping for fast message lookups
-- ═══════════════════════════════════════════════════════════

CREATE TEMP TABLE EntryMap (
    AccountId  INTEGER NOT NULL,
    UserId     INTEGER NOT NULL,
    EntryId    INTEGER NOT NULL,
    Kind       VARCHAR(1) NOT NULL,
    EntryDisplayName TEXT NOT NULL,
    PRIMARY KEY (AccountId, UserId)
);

INSERT INTO EntryMap (AccountId, UserId, EntryId, Kind, EntryDisplayName)
SELECT ce.AccountId, ce.UserId, e.Id, ce.Kind, e.DisplayName
FROM CleanEntry ce
JOIN Entry e ON e.Account = ce.AccountId
           AND e.HumanReadableId = ce.HumanReadableId;


-- ═══════════════════════════════════════════════════════════
-- 5. Messages
-- ═══════════════════════════════════════════════════════════
-- The AccountNick CTE pre-computes a display nick per account
-- to use as the outgoing-message sender name.
--
-- DisplayName mapping (per-message sender name):
--   MUC messages      → old Variant (= sender's nick / MUC occupant resource),
--                        falling back to the account nick where Variant is empty.
--   Chat incoming     → VisibleName from azoth_entrycache, then Entry.DisplayName.
--   PrivateChat in    → Entry.DisplayName directly (entrycache has full JID, not nick).
--   All outgoing      → account-derived nick.
--
-- Variant mapping:
--   MUC messages  → NULL (old Variant was the nick, now in DisplayName)
--   Chat messages → old Variant as-is (XMPP resource / protocol-specific variant)
--
-- Direction:
--   Old "IN"/"OUT" strings → new 'I'/'O' single chars.
--
-- RichBody:
--   NULLIF converts empty strings to NULL (matches std::optional<QString> semantics).

WITH AccountNick AS (
    SELECT Id,
        CASE
            WHEN AccountID LIKE 'Xoox.Gloox.XMPP_%@vk.com' THEN 'Me'
            WHEN AccountID LIKE 'Xoox.Gloox.XMPP_%@%' THEN
                SUBSTR(AccountID,
                       INSTR(AccountID, '_') + 1,
                       INSTR(AccountID, '@') - INSTR(AccountID, '_') - 1)
            ELSE (SELECT Value FROM Config WHERE Key = 'FallbackNick')
        END AS Nick
    FROM old.azoth_accounts
)
INSERT INTO Message (Entry, DisplayName, Variant, PersistentId,
                     TS, Direction, Body, RichBody)
SELECT
    em.EntryId,
    -- DisplayName
    CASE
        WHEN em.Kind = 'M' THEN COALESCE(NULLIF(h.Variant, ''), an.Nick)
        WHEN em.Kind = 'P' AND h.Direction = 'IN' THEN em.EntryDisplayName
        WHEN h.Direction = 'IN' THEN COALESCE(
            (SELECT ec.VisibleName FROM old.azoth_entrycache ec WHERE ec.Id = h.Id),
            em.EntryDisplayName)
        ELSE an.Nick
    END,
    -- Variant
    -- Old schema stored the *remote* resource in Variant for all message types.
    -- New schema redefines Variant as the *local* (our) resource for non-MUC messages,
    -- which we don't have in the old data, so outgoing Variant is always NULL.
    -- MUC Variant is already NULL (old Variant was the sender nick, now in DisplayName).
    CASE WHEN em.Kind = 'M' THEN NULL
         WHEN h.Direction = 'OUT' THEN NULL
         ELSE NULLIF(h.Variant, '')
    END,
    -- PersistentId
    NULL,
    -- TS
    h.Date,
    -- Direction
    CASE WHEN h.Direction = 'IN' THEN 'I' ELSE 'O' END,
    -- Body
    h.Message,
    -- RichBody
    NULLIF(h.RichMessage, '')
FROM old.azoth_history h
JOIN EntryMap em ON em.AccountId = h.AccountId AND em.UserId = h.Id
JOIN AccountNick an ON an.Id = em.AccountId
WHERE h.Type IN ('CHAT', 'MUC')
ORDER BY h.rowid;


DROP TABLE EntryMap;
DROP TABLE CleanEntry;
DROP TABLE Config;

COMMIT;

DETACH DATABASE old;

-- Restore safe defaults (matching Storage2 constructor PRAGMAs).
PRAGMA foreign_keys = ON;
PRAGMA synchronous = NORMAL;
PRAGMA journal_mode = WAL;
PRAGMA case_sensitive_like = ON;


-- ═══════════════════════════════════════════════════════════
-- Validation queries (uncomment to run manually)
-- ═══════════════════════════════════════════════════════════
-- .headers on
-- .mode column
--
-- -- Row counts
-- SELECT 'Account'  AS tbl, COUNT(*) AS cnt FROM Account
-- UNION ALL
-- SELECT 'Entry',           COUNT(*)         FROM Entry
-- UNION ALL
-- SELECT 'MucContext',      COUNT(*)         FROM MucContext
-- UNION ALL
-- SELECT 'Message',         COUNT(*)         FROM Message;
--
-- -- Compare message count with old DB (attach it first)
-- -- ATTACH 'history.db' AS old;
-- -- SELECT
-- --   (SELECT COUNT(*) FROM old.azoth_history WHERE Type IN ('CHAT','MUC')) AS old_msgs,
-- --   (SELECT COUNT(*) FROM Message) AS new_msgs;
--
-- -- Spot-check: accounts
-- SELECT Id, AccountName FROM Account;
--
-- -- Spot-check: entry kinds and display names
-- SELECT Kind, DisplayName, HumanReadableId FROM Entry ORDER BY Kind LIMIT 30;
--
-- -- Spot-check: MUC contexts
-- SELECT e.HumanReadableId, e.DisplayName, mc.MucName, e.Kind
-- FROM MucContext mc JOIN Entry e ON e.Id = mc.Id LIMIT 20;
--
-- -- Spot-check: no PrivateChat DisplayNames with '/' (should be just nicks)
-- SELECT DisplayName, HumanReadableId FROM Entry WHERE Kind = 'P' AND DisplayName LIKE '%/%';
--
-- -- Count entries dropped due to unrecognizable EntryID format
-- -- ATTACH 'history.db' AS old;
-- -- SELECT COUNT(DISTINCT h.Id || '-' || h.AccountId) AS dropped_entries
-- -- FROM old.azoth_history h
-- -- JOIN old.azoth_users u ON u.Id = h.Id
-- -- JOIN old.azoth_accounts a ON a.Id = h.AccountId
-- -- WHERE h.Type IN ('CHAT', 'MUC')
-- --   AND SUBSTR(u.EntryID, 1, LENGTH(a.AccountID) + 1) != a.AccountID || '_';
