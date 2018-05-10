# CMake generated Testfile for 
# Source directory: /users/kanwu/project/btree/test
# Build directory: /users/kanwu/project/btree/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_fs "./test_fs")
add_test(test_btree "./test_btree")
add_test(test_fs_valgrind "valgrind" "--error-exitcode=1" "--read-var-info=yes" "--leak-check=full" "--show-leak-kinds=all" "./test_fs")
add_test(test_btree_valgrind "valgrind" "--error-exitcode=1" "--read-var-info=yes" "--leak-check=full" "--show-leak-kinds=all" "./test_btree")
