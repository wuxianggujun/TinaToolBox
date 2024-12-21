cmake_minimum_required(VERSION 3.18)
project(libcalamine)

# --- 导入 Rust 库 ---
corrosion_import_crate(MANIFEST_PATH ./Cargo.toml PROFILE release CRATES libcalamine FEATURES default)

#[[configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
configure_file(${PROJECT_NAME}-static.pc.in ${PROJECT_NAME}-static.pc @ONLY)]]

# corrosion doesn't supporting installing libraries yet
# https://github.com/corrosion-rs/corrosion/issues/415
# so we'll do it ourselves
#[[
include(GNUInstallDirs)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/libextism.a
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/libextism.so
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        OPTIONAL
)
install( FILES ../runtime/extism.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-static.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)]]
