

aux_source_directory(. RootSrc)

add_library(DexLoader SHARED ${RootSrc})
target_link_libraries(DexLoader android dl z AndroidDef FAInHook)
