find_package (Python)

set(symlinkPy ${CMAKE_CURRENT_LIST_DIR}/symlink.py)

function(symlink src dst)
  execute_process(COMMAND ${Python_EXECUTABLE} ${symlinkPy}
    "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
    "${CMAKE_CURRENT_BINARY_DIR}/${dst}")
endfunction()

