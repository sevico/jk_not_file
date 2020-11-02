project(FShell)
SET(FA_STATIC ON)

add_subdirectory(AndroidDef)
add_subdirectory(DexLoader)
add_subdirectory(FAInHook)
aux_source_directory(. RootSrc)
add_library(FShell SHARED ${RootSrc})
target_link_libraries(FShell android dl z AndroidDef DexLoader)
