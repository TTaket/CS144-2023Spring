# CMake generated Testfile for 
# Source directory: /home/ljy/LJY/code-dir/CS144
# Build directory: /home/ljy/LJY/code-dir/CS144/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[compile with bug-checkers]=] "/usr/bin/cmake" "--build" "/home/ljy/LJY/code-dir/CS144/build" "-t" "functionality_testing" "webget")
set_tests_properties([=[compile with bug-checkers]=] PROPERTIES  FIXTURES_SETUP "compile" TIMEOUT "-1" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;6;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
add_test([=[t_webget]=] "/home/ljy/LJY/code-dir/CS144/tests/webget_t.sh" "/home/ljy/LJY/code-dir/CS144/build")
set_tests_properties([=[t_webget]=] PROPERTIES  FIXTURES_REQUIRED "compile" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;17;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
add_test([=[byte_stream_basics]=] "/home/ljy/LJY/code-dir/CS144/build/tests/byte_stream_basics_sanitized")
set_tests_properties([=[byte_stream_basics]=] PROPERTIES  FIXTURES_REQUIRED "compile" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;10;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;20;ttest;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
add_test([=[byte_stream_capacity]=] "/home/ljy/LJY/code-dir/CS144/build/tests/byte_stream_capacity_sanitized")
set_tests_properties([=[byte_stream_capacity]=] PROPERTIES  FIXTURES_REQUIRED "compile" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;10;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;21;ttest;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
add_test([=[byte_stream_one_write]=] "/home/ljy/LJY/code-dir/CS144/build/tests/byte_stream_one_write_sanitized")
set_tests_properties([=[byte_stream_one_write]=] PROPERTIES  FIXTURES_REQUIRED "compile" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;10;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;22;ttest;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
add_test([=[byte_stream_two_writes]=] "/home/ljy/LJY/code-dir/CS144/build/tests/byte_stream_two_writes_sanitized")
set_tests_properties([=[byte_stream_two_writes]=] PROPERTIES  FIXTURES_REQUIRED "compile" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;10;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;23;ttest;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
add_test([=[byte_stream_many_writes]=] "/home/ljy/LJY/code-dir/CS144/build/tests/byte_stream_many_writes_sanitized")
set_tests_properties([=[byte_stream_many_writes]=] PROPERTIES  FIXTURES_REQUIRED "compile" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;10;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;24;ttest;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
add_test([=[byte_stream_stress_test]=] "/home/ljy/LJY/code-dir/CS144/build/tests/byte_stream_stress_test_sanitized")
set_tests_properties([=[byte_stream_stress_test]=] PROPERTIES  FIXTURES_REQUIRED "compile" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;10;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;25;ttest;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
add_test([=[compile with optimization]=] "/usr/bin/cmake" "--build" "/home/ljy/LJY/code-dir/CS144/build" "-t" "speed_testing")
set_tests_properties([=[compile with optimization]=] PROPERTIES  FIXTURES_SETUP "compile_opt" TIMEOUT "-1" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;36;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
add_test([=[byte_stream_speed_test]=] "/home/ljy/LJY/code-dir/CS144/build/tests/byte_stream_speed_test")
set_tests_properties([=[byte_stream_speed_test]=] PROPERTIES  FIXTURES_REQUIRED "compile_opt" _BACKTRACE_TRIPLES "/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;40;add_test;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;47;stest;/home/ljy/LJY/code-dir/CS144/etc/tests.cmake;0;;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;16;include;/home/ljy/LJY/code-dir/CS144/CMakeLists.txt;0;")
subdirs("util")
subdirs("src")
subdirs("tests")
subdirs("apps")
