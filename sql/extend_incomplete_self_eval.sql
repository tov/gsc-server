UPDATE submission
SET
  eval_date = GREATEST(eval_date, utc_now()) + INTERVAL '24 hrs'
WHERE
  submission_size(id) < assignment_size(assignment_number)
AND
  effective_eval_date(id) < utc_now() + INTERVAL '12 hrs';
