#!/bin/bash

set -e
HERE=$(cd $(dirname $0); pwd)

cd $HERE

bundle exec haml -r yen -Eutf-8 docsrc/calcproc.haml > htmldoc/calcproc.html
