#!/bin/sh

cd "$(dirname $0)"/..

if [ $# != 1 ]; then
    echo >&2 "Usage: $0 ADMIN_PASSWORD"
    exit 1
fi

ADMIN_PASSWORD=$1

# Require password up front
echo >&2 "Running sudo..."
sudo true

# Need gsc-createdb to create the tables and set the
# password.
bin/build.sh debug gsc-createdb

# Stop Apache, which stops access to the database
sudo service apache2 stop

# Drop and recreate the database
sudo -u postgres dropdb gsc
sudo -u postgres createdb gsc

# Start GSC to create tables and set password
sudo -u gsc ADMIN_PASSWORD="$ADMIN_PASSWORD" build.debug/gsc-createdb

# Restart apache
sudo service apache2 start
