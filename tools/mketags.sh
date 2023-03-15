#! /bin/sh

ctags -e -a `find . -type f -name \*.[hcS]`
for d in `find . -t