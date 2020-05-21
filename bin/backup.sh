#!/bin/sh

set -e

dir=gsc-backups
localdir=$HOME/$dir
file=cs211-$(date +%Y%m%d-%H%M%S).psql
host=login.eecs.northwestern.edu
gpg_recipient=jesse.tov@gmail.com
gpg_public_key=0xC5668BA5047AE6BD

if [ "$(date +%N)" = N ]; then
    date_fmt=%s
    time_fmt=%ds
else
    date_fmt=%s.%N
    time_fmt=%.2fs
fi

gpgz () {
    gpg --compress-level 7 --output - "$@"
}

time_stamp () {
    date +"$date_fmt"
}

reset_timer () {
    timer_base=$(time_stamp)
}

elapsed_time () {
    echo "$(time_stamp) - $timer_base" | bc -l
}

PROGRESS="\r%s... $time_fmt "
COMPLETE="\r%s... done ($time_fmt).\n"
STEP_START=256
STEP_MAX=1048576
STEP_FACTOR=2

progress_meter () (
    exec >&2

    step_size=$STEP_START

    reset_timer

    while [ -n "$(head -c $step_size | tr '\0-\177' x)" ]; do
        printf "$PROGRESS" "$1" $(elapsed_time)

        test $step_size -gt $STEP_MAX ||
            : $(( step_size *= STEP_FACTOR ))
    done

    printf "$COMPLETE" "$1" $(elapsed_time)
)

progress_output () {
    local descr; descr=$1; shift
    local target; target=$1; shift

    "$@" | tee "$target" | progress_meter "$descr"
}

progress_around () {
    local descr; descr=$1; shift

    reset_timer
    printf>&2 "$PROGRESS" "$descr" 0
    "$@"
    printf>&2 "$COMPLETE" "$descr" $(elapsed_time)
}

mkdir -p $localdir
cd $localdir

progress_around "Authenticating" \
    sudo -u gsc true

progress_output "Dumping database" $file \
    sudo -u gsc pg_dump gsc

progress_output "Encrypting" $file.gpg \
    gpgz --sign --recipient "$gpg_recipient" --encrypt $file

rm $file

progress_around "Synchronizing" \
    rsync --times --human-readable --progress \
        $localdir/*.gpg $host:$dir/
