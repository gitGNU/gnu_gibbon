#! /bin/sh

dir=${srcdir:='.'}

./test_match_complete $dir/maxfill-white-incomplete.gmd \
        $dir/maxfill-white-complete.gmd
./test_match_complete $dir/maxfill-black-incomplete.gmd \
        $dir/maxfill-black-complete.gmd
