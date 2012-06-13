#! /bin/sh

dir=${srcdir:='.'}
./test_match_complete $dir/incomplete-opening.gmd \
        $dir/complete-opening.gmd
./test_match_complete $dir/incomplete-initial-resign.gmd \
        $dir/complete-initial-resign.gmd
