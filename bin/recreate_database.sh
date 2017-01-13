#!/bin/sh

cd "$(dirname $0)"/..

if [ -z "$1" -o -n "$2" ]; then
    echo >&2 Usage: $0 ADMIN_PASSWORD
    exit 1
fi

# Need a debug (WtHTTP) version of gsc to create the tables and set the
# password.
bin/build-debug.sh

# Stop Apache, which stops access to the database
sudo service apache2 stop

# Drop and recreate the database
sudo -u postgres dropdb gsc
sudo -u postgres createdb gsc

# Start GSC to create tables and set password
sudo -u gsc bin/gsc-http.sh &
PID=$!
sleep 1
sudo kill $PID

# Restart apache
sudo service apache2 start
