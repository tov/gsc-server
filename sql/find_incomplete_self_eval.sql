SELECT
  assignment_number AS asst,
  submitter_name(id) AS submitter,
  assignment_size(assignment_number) - submission_size(id) AS missing,
  effective_eval_date(id) AS due
FROM submission
WHERE submission_size(id) < assignment_size(assignment_number)
  AND effective_eval_date(id) <= utc_now() + INTERVAL '12 hrs'
ORDER BY asst, submitter;
