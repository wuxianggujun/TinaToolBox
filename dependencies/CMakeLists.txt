add_subdirectory(QXlsx/QXlsx)
add_subdirectory(tomlplusplus)

# 在 add_subdirectory 之前设置选项
set(CPPTERMINAL_BUILD_EXAMPLES OFF)
set(CPPTERMINAL_ENABLE_TESTING OFF)
set(CPPTERMINAL_ENABLE_DOCS OFF)

add_subdirectory(cpp-terminal)