#! /bin/sh

dir=${srcdir:='.'}

./test_match_complete $dir/maxfill-white-incomplete.gmd \
        $dir/maxfill-white-complete.gmd || exit 1
./test_match_complete $dir/maxfill-black-incomplete.gmd \
        $dir/maxfill-black-complete.gmd || exit 1
./test_match_complete $dir/drop-white-incomplete.gmd \
        $dir/drop-white-complete.gmd || exit 1
./test_match_complete $dir/drop-black-incomplete.gmd \
        $dir/drop-black-complete.gmd || exit 1
./test_match_complete $dir/roll-white-incomplete.gmd \
        $dir/roll-white-complete.gmd || exit 1
./test_match_complete $dir/roll-black-incomplete.gmd \
        $dir/roll-black-complete.gmd || exit 1
./test_match_complete $dir/reject-white-incomplete.gmd \
        $dir/reject-white-complete.gmd || exit 1
./test_match_complete $dir/reject-black-incomplete.gmd \
        $dir/reject-black-complete.gmd || exit 1
./test_match_complete $dir/reject2-white-incomplete.gmd \
        $dir/reject2-white-complete.gmd || exit 1
./test_match_complete $dir/reject2-black-incomplete.gmd \
        $dir/reject2-black-complete.gmd || exit 1