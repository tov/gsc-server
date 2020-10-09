#!/bin/sh

log=/var/log/gsc-cron.log
exec >>$log 2>&1

date

echo Create grader eval for HW2 item 3:
bin/create_grader_evals.sh 10 1 1 'freebie :)'
