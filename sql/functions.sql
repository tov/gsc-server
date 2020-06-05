-- Returns the current time in UTC.
CREATE OR REPLACE FUNCTION
  utc_now()
RETURNS TIMESTAMP WITHOUT TIME ZONE
AS $$
  SELECT NOW() AT TIME ZONE 'utc'
$$
LANGUAGE SQL;

-- Returns the effective code deadline for the submission with the
-- given id.
CREATE OR REPLACE FUNCTION
  effective_due_date(BIGINT)
RETURNS TIMESTAMP WITHOUT TIME ZONE
AS $$
  SELECT GREATEST(s.due_date, a.due_date)
  FROM submission AS s
  INNER JOIN assignment AS a
  ON s.assignment_number = a.number
  WHERE s.id = $1
$$
LANGUAGE SQL;

-- Returns the effective self-eval deadline for the submission with the
-- given id.
CREATE OR REPLACE FUNCTION
  effective_eval_date(BIGINT)
RETURNS TIMESTAMP WITHOUT TIME ZONE
AS $$
  SELECT GREATEST(s.eval_date, a.eval_date)
  FROM submission AS s
  INNER JOIN assignment AS a
  ON s.assignment_number = a.number
  WHERE s.id = $1
$$
LANGUAGE SQL;

-- Returns whether the due date of the submission with the given id is past.
CREATE OR REPLACE FUNCTION
  is_past_due_date(BIGINT)
RETURNS BOOLEAN
AS $$
  SELECT effective_due_date($1) < utc_now()
$$
LANGUAGE SQL;

-- Returns whether the eval date of the submission with the given id is past.
CREATE OR REPLACE FUNCTION
  is_past_eval_date(BIGINT)
RETURNS BOOLEAN
AS $$
  SELECT effective_eval_date($1) < utc_now()
$$
LANGUAGE SQL;


-- Returns user name of the first owner of the submission with the
-- given id.
CREATE OR REPLACE FUNCTION
  submitter_name(BIGINT)
RETURNS VARCHAR
AS $$
  SELECT u.name
  FROM submission AS s
  INNER JOIN gsc_user AS u
  ON s.user1_id = u.id
  WHERE s.id = $1
$$
LANGUAGE SQL;

-- Returns the number of evaluation items for the assignment of the
-- given number.
CREATE OR REPLACE FUNCTION
  assignment_size(BIGINT)
RETURNS BIGINT
AS $$
  SELECT COUNT(*)
  FROM eval_item
  WHERE assignment_number = $1
$$
LANGUAGE SQL;

-- Returns the number of self-evaluation items for the submission
-- of the given id.
CREATE OR REPLACE FUNCTION
  submission_size(BIGINT)
RETURNS BIGINT
AS $$
  SELECT COUNT(*)
  FROM self_eval
  WHERE submission_id = $1
$$
LANGUAGE SQL;
