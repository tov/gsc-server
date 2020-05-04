CREATE OR REPLACE FUNCTION
  utc_now()
RETURNS TIMESTAMP WITHOUT TIME ZONE
AS $$
  SELECT NOW() AT TIME ZONE 'utc'
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION
  effective_eval_date(submission_id BIGINT)
RETURNS TIMESTAMP WITHOUT TIME ZONE
AS $$
  SELECT GREATEST(s.eval_date, a.eval_date)
  FROM submission AS s
  INNER JOIN assignment AS a
  ON s.assignment_number = a.number
  WHERE s.id = submission_id
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION
  submitter_name(submission_id BIGINT)
RETURNS VARCHAR
AS $$
  SELECT u.name
  FROM submission AS s
  INNER JOIN gsc_user AS u
  ON s.user1_id = u.id
  WHERE s.id = submission_id
$$
LANGUAGE SQL;

SELECT
  s.assignment_number,
  submitter_name(s.id) AS submitter,
  effective_eval_date(s.id)
FROM
  submission AS s
LEFT OUTER JOIN
  self_eval AS e
ON
  e.submission_id = s.id
WHERE
  e IS NULL
AND
  effective_eval_date(s.id) <= utc_now() + INTERVAL '12 hrs'
ORDER BY
  s.assignment_number,
  submitter
;
