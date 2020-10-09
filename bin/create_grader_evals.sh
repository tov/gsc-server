#!/bin/sh

# Create a grader_eval for every self evaluation of the specified eval
# item.
if ! (( $# )); then
    echo>&2 "Usage: $0 EVAL_ITEM_ID [GRADER_ID [SCORE [COMMENT]]]"
    exit 1
fi

eval_item_id=$1
grader_id=${2:-1}
score=${3:-1}
comment="$(printf '%s\n' "${4-}" | sed "s/'/''/g")"

as_gsc () {
    if [ `whoami` != gsc ]; then
        sudo -u gsc "$@"
    else
        "$@"
    fi
}

as_gsc psql -c "
    INSERT INTO grader_eval (
        version,
        self_eval_id,
        grader_id,
        status,
        explanation,
        score,
        time_stamp
    ) SELECT
        0,
        self_eval.id,
        $grader_id,
        2,
        '$comment',
        $score,
        utc_now()
    FROM self_eval
        LEFT OUTER JOIN grader_eval ON self_eval_id = self_eval.id
    WHERE eval_item_id = $eval_item_id
      AND grader_eval.id IS NULL;
"
