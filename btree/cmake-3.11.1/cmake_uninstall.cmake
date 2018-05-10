if(NOT EXISTS "/users/kanwu/external_btree/cmake-3.11.1/install_manifest.txt")
  message(FATAL_ERROR "Cannot find install manifest: \"/users/kanwu/external_btree/cmake-3.11.1/install_manifest.txt\"")
endif()

file(READ "/users/kanwu/external_btree/cmake-3.11.1/install_manifest.txt" files)
string(REPLACE "\n" ";" files "${files}")
foreach(file ${files})
  message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
  if(EXISTS "$ENV{DESTDIR}${file}")
    exec_program(
      "/users/kanwu/external_btree/cmake-3.11.1/Bootstrap.cmk/cmake" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
      OUTPUT_VARIABLE rm_out
      RETURN_VALUE rm_retval
      )
    if("${rm_retval}" STREQUAL 0)
    else()
      message(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    endif()
  else()
    message(STATUS "File \"$ENV{DESTDIR}${file}\" does not exist.")
  endif()
endforeach()
