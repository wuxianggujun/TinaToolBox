add_subdirectory(QXlsx/QXlsx)
add_subdirectory(tomlplusplus)

# 在 add_subdirectory 之前设置选项
set(CPPTERMINAL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CPPTERMINAL_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(CPPTERMINAL_ENABLE_DOCS OFF CACHE BOOL "" FORCE)
 
add_subdirectory(cpp-terminal)

add_subdirectory(OpenXLSX)

add_subdirectory(utfcpp)

#add_subdirectory(libcalamine)