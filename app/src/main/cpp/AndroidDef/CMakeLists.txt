cmake_minimum_required(VERSION 3.4.1)

IF (WIN32 OR WIN64)
    MESSAGE("Building windows version, SysUtil can not used.")
ELSE()
    add_definitions(-DHAVE_POSIX_FILEMAP)
ENDIF ()

aux_source_directory(libdex SRC_LIBDEX)
aux_source_directory(dexdump SRC_DEXDUMP)
aux_source_directory(vm SRC_VM)
aux_source_directory(vm/oo SRC_VM_OO)
aux_source_directory(vm/native SRC_VM_NATIVE)



set(SOURCE_FILES ${SRC_LIBDEX} ${SRC_DEXDUMP} ${SRC_VM} ${SRC_VM_OO} ${SRC_VM_NATIVE} BaseDef.cpp)
add_library(AndroidDef STATIC ${SOURCE_FILES})

if(CMAKE_SYSTEM_NAME MATCHES "Android")
    target_link_libraries(AndroidDef z log)
else()
    target_link_libraries(AndroidDef z)
endif()

target_include_directories(AndroidDef PUBLIC .)
