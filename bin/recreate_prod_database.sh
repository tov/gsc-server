#!/bin/sh

set -eu

cd "$(dirname $0)"/..

: ${admin_password:=}
: ${dropdb_flags:=}
: ${deploy_type:=test}
: ${db_name:=gsc}
: ${db_user:=gsc}
: ${gsc_createdb_flags:=-c}

main () {
    process_args "$@"

    # Set up Postgres environment
    export POSTGRES_CONNINFO=dbname=$db_name
    export ADMIN_PASSWORD=$admin_password

    # Require password up front
    if_deploy echo "Running sudo..." >&2
    if_deploy sudo true

    # Need gsc-createdb to create the tables and set the
    # password.
    bin/build.sh debug gsc-createdb

    # Stop Apache, which stops access to the database
    if_deploy sudo service apache2 stop

    # Drop and recreate the database
    as_postgres_user dropdb $dropdb_flags $db_name
    as_postgres_user createdb $db_name

    # Start GSC to create tables and set password
    (
    cd server_root
    as_db_user ../build.debug/gsc-createdb $gsc_createdb_flags
    )

    # Load stored procedures
    as_db_user psql $db_name < sql/functions.sql

    # Restart apache
    if_deploy sudo service apache2 start
}

as_db_user () {
    if [ $deploy_type = dev ]; then
        "$@"
    else
        sudo -u $db_user "$@"
    fi
}

as_postgres_user () {
    if [ $deploy_type = dev ]; then
        "$@"
    else
        sudo -u postgres "$@"
    fi
}

if_deploy () {
    if [ $deploy_type != dev ]; then
        "$@"
    fi
}

process_args () {
    while [ $# -gt 0 ]; do
        case $1 in
            (-h|--help)
                usage
                exit 0
                ;;

            (-c)
                dropdb_flags=--if-exists
                shift
                ;;

            (-d)
                deploy_type=dev
                shift
                ;;

            (-t)
                deploy_type=test
                shift
                ;;

            (-p)
                deploy_type=prod
                shift
                ;;

            (-a)
                admin_password="$2"
                shift 2
                ;;

            (-a*)
                admin_password="${1#-a}"
                shift
                ;;

            (-n)
                db_name="$2"
                shift 2
                ;;

            (-n*)
                db_name="${1#-n}"
                shift
                ;;

            (--)
                shift
                ;;

            (*)
                exec >&2
                echo "$0: error: unrecognized argument: $1"
                usage
                exit 1
                ;;
        esac
    done

    if [ $deploy_type = prod ]; then
        if [ -z "$admin_password" ]; then
            exec >&2
            echo "$0: error: prod admin password cannot be blank"
            usage
            exit 2
        fi

        gsc_createdb_flags=
    fi
}

usage () {
    echo "Usage: $0 [-d|-t|-p] [-a ADMIN_PASSWORD] [-c] [-n DB_NAME]"
}

#########
main "$@"
#########
