#!/bin/bash

set -e
HERE=$(cd $(dirname $0); pwd)

cd $HERE

bundle exec haml -Eutf-8 docsrc/calcproc.haml > htmldoc/calcproc.html
