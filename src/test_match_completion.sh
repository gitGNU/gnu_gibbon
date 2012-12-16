#! /bin/sh

dir=${srcdir:='.'}
exit_code=0

./test_match_complete $dir/maxfill-white-incomplete.gmd \
        $dir/maxfill-white-complete.gmd || exit_code=1
./test_match_complete $dir/maxfill-black-incomplete.gmd \
        $dir/maxfill-black-complete.gmd || exit_code=1
./test_match_complete $dir/drop-white-incomplete.gmd \
        $dir/drop-white-complete.gmd || exit_code=1
./test_match_complete $dir/drop-black-incomplete.gmd \
        $dir/drop-black-complete.gmd || exit_code=1
./test_match_complete $dir/roll-white-incomplete.gmd \
        $dir/roll-white-complete.gmd || exit_code=1
./test_match_complete $dir/roll-black-incomplete.gmd \
        $dir/roll-black-complete.gmd || exit_code=1
./test_match_complete $dir/reject-white-incomplete.gmd \
        $dir/reject-white-complete.gmd || exit_code=1
./test_match_complete $dir/reject-black-incomplete.gmd \
        $dir/reject-black-complete.gmd || exit_code=1
./test_match_complete $dir/reject2-white-incomplete.gmd \
        $dir/reject2-white-complete.gmd || exit_code=1
./test_match_complete $dir/reject2-black-incomplete.gmd \
        $dir/reject2-black-complete.gmd || exit_code=1
./test_match_complete $dir/initial-with-game.gmd \
        $dir/opening-white-complete.gmd || exit_code=1
./test_match_complete $dir/initial-with-game.gmd \
        $dir/opening-black-complete.gmd || exit_code=1
./test_match_complete $dir/initial.gmd \
        $dir/opening-white-complete.gmd || exit_code=1
./test_match_complete $dir/initial.gmd \
        $dir/opening-black-complete.gmd || exit_code=1
./test_match_complete $dir/initial-with-double.gmd \
        $dir/opening-white-double-complete.gmd || exit_code=1
./test_match_complete $dir/initial-with-double.gmd \
        $dir/opening-black-double-complete.gmd || exit_code=1
./test_match_complete $dir/initial-with-one-double.gmd \
        $dir/initial-with-two-doubles.gmd || exit_code=1
./test_match_complete $dir/dance-incomplete.gmd \
        $dir/dance-complete0.gmd || exit_code=1
./test_match_complete $dir/dance-incomplete.gmd \
        $dir/dance-complete1.gmd || exit_code=1
./test_match_complete $dir/dance-incomplete.gmd \
        $dir/dance-complete2.gmd || exit_code=1
./test_match_complete $dir/win-by-move.gmd \
        $dir/win-by-move-complete.gmd || exit_code=1
./test_match_complete $dir/win-by-double.gmd \
        $dir/win-by-double-complete.gmd || exit_code=1
./test_match_complete $dir/win-by-resignation.gmd \
        $dir/win-by-resignation-complete.gmd || exit_code=1

exit $exit_code
