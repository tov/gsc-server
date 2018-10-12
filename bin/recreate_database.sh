#!/bin/sh

cd "$(dirname $0)"/..

if [ -z "$1" -o -n "$2" ]; then
    echo >&2 Usage: $0 ADMIN_PASSWORD
    exit 1
fi

ADMIN_PASSWORD="$1"

# Require password up front
sudo true

# Need a debug (WtHTTP) version of gsc to create the tables and set the
# password.
bin/build.sh debug

# Stop Apache, which stops access to the database
sudo service apache2 stop

# Drop and recreate the database
sudo -u postgres dropdb gsc
sudo -u postgres createdb gsc

# Start GSC to create tables and set password
sudo -u gsc ADMIN_PASSWORD="$ADMIN_PASSWORD" bin/gsc-http.sh &
echo "Started gscd, waiting 5 seconds"
sleep 5
sudo killall gscd

# Restart apache
sudo service apache2 start
