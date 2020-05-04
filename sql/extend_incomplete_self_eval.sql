UPDATE submission
SET eval_date = utc_now() + INTERVAL '24 hrs'
WHERE id IN (
  SELECT s.id
  FROM submission AS s
  LEFT OUTER JOIN self_eval AS e ON e.submission_id = s.id
  WHERE e IS NULL
    AND effective_eval_date(s.id) <= utc_now() + INTERVAL '12 hrs'
);
