add_library(psg_mml INTERFACE)
target_sources(psg_mml INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/psg_mml.c
    ${CMAKE_CURRENT_LIST_DIR}/src/local/psg_mml_fifo.c
    ${CMAKE_CURRENT_LIST_DIR}/src/local/psg_mml_decode.c
    ${CMAKE_CURRENT_LIST_DIR}/src/local/psg_mml_ctrl.c
    ${CMAKE_CURRENT_LIST_DIR}/src/local/psg_mml_utils.c
)
target_include_directories(psg_mml INTERFACE 
    ${PROJECT_SOURCE_DIR}/inc
)

