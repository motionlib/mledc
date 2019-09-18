#!/bin/bash

set -e
HERE=$(cd $(dirname $0); pwd)

cd $HERE/docsrc

bundle exec haml -r ./calcproc -r yen -Eutf-8 calcproc.haml > ../htmldoc/calcproc.html
