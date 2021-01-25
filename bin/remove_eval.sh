#!/bin/sh

main () {
    process_args "$@"

    psql $db -c "
        DELETE FROM self_eval
        WHERE id = (
            SELECT s.id
            FROM gsc_user AS u
            INNER JOIN submission AS b ON (u.id = b.user1_id)
            INNER JOIN self_eval AS s ON (s.submission_id = b.id)
            INNER JOIN eval_item AS i ON (s.eval_item_id = i.id)
            WHERE u.name = '$user'
            AND i.sequence = $item
            AND i.assignment_number = $hw
        );
    "
}

process_args () {
    if [ $# != 4 ]
    then
        usage >&2
        exit 1
    fi

    db=$1
    hw=$2
    user=$3
    item=$4
}

usage () {
    echo "Usage: ${0##*/} DB HW USER ITEM"
}

#########
main "$@"
#########
