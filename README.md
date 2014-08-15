MariaDB package dthread support demonstration
=============================================

This is a Duda web service that demonstrates the usage of newly added dthread
support for mariadb package.

## Introduction ##
This web service is similar to [mariadb demo](https://github.com/swpd/duda_mariadb_demo)
except that it is done using dthread apis. Through the new apis the web service
is rewritten in a synchronous-like manner, which makes it more programmer friendly.

## Installation ##
clone the latest Monkey HTTP Daemon repository from Github:

    git clone https://github.com/monkey/monkey.git

change to Monkey's plugins directory:

    cd monkey/plugins

clone Duda from my Github and check out `mariadb_dthread` branch:

    git clone https://github.com/swpd/duda.git

    cd duda

    git checkout mariadb_dthread

create lib directory for duda:

    mkdir lib

    cd lib

clone this repository from my Github:

    git clone https://github.com/swpd/duda_mariadb_dthread.git mariadb_dthread

configure from monkey root directory with duda plugin enabled:

    ./configure --enable-plugins=duda

if you prefer verbose message output of monkey, configure it with `--trace` option
(this option should not be used in a production environment):

    ./configure --enable-plugins=duda --trace

build Monkey(go get yourself a cup of coffee, it might take a while):

    make

Notice: The MariaDB client library is compiled from source because it may be
unavailable on some distribution. And it requires `cmake` and `libaio` to be
installed on your machine:

    Debian/Ubuntu              : apt-get install libaio-dev cmake
    RedHat/Fedora/Oracle Linux : yum install libaio-devel cmake
    SUSE                       : zypper install libaio-devel cmake

edit `conf/plugins.load` to make sure `duda` is enabled:

    Load /path/to/monkey-duda.so

edit `conf/duda/duda.conf` and change services root:

    ServicesRoot /path/to/monkey/plugins/duda/lib

edit `conf/sites/default` to add this web service:

    [WEB_SERVICE]
        Name mariadb_dthread
        Enabled on

run the server:

    bin/monkey

### Serving Requests ###
use your favorite client to visit the following URL to access this web service:

    http://localhost:2001/mariadb_dthread/mariadb/home/
