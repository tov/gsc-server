#!/bin/sh

# Extend every incomplete self evaluation that’s due in 12 hours or less
# by at least 24 hours and to at least 24 hours from now.

log=/var/log/autoextend.log
exec >>$log 2>&1

date

psql -c "
    UPDATE submission
    SET eval_date = GREATEST(eval_date, utc_now()) + INTERVAL '24 hrs'
    WHERE submission_size(id) < assignment_size(assignment_number)
      AND effective_eval_date(id) < utc_now() + INTERVAL '12 hrs';
"

