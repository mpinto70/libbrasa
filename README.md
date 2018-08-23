# libbrasa
Library of utilities that I recurrently need and have to recode.

If you want to install `libbrasa` as an external project in your project you could add the
following:
```
###################################
# libbrasa - BEGIN ################
###################################

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/external)

ExternalProject_Add(libbrasa
    GIT_REPOSITORY https://github.com/mpinto70/libbrasa.git
    GIT_TAG v0.4
    PREFIX ${CMAKE_BINARY_DIR}/brasa
    BUILD_COMMAND ""
    INSTALL_COMMAND
        make
        DESTDIR=${EXTERNAL_INSTALL_LOCATION}
        install
    LOG_INSTALL ON
    LOG_BUILD ON
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/brasa/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/brasa/lib)

###################################
# libbrasa - END   ################
###################################
```
