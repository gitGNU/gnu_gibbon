#! /bin/sh

dir=${srcdir:='.'}
./test_match_complete $dir/incomplete-opening.gmd $dir/complete-opening.gmd
./test_match_complete $dir/incomplete-opening.gmd $dir/complete-opening.gmd 1
./test_match_complete $dir/incomplete-opening.gmd $dir/complete-opening.gmd 2
