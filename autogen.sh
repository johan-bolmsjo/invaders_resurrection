#!/bin/sh

aclocal
autoheader
automake --foreign -a
autoconf
