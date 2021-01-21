BEGIN;

ALTER TABLE assignment
ADD points INTEGER;

UPDATE assignment SET points = 0;

ALTER TABLE assignment
ALTER COLUMN points SET NOT NULL;

COMMIT;
