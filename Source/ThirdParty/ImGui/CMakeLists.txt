set (TARGET_NAME ImGui)

file (GLOB CPP_FILES src/*.cpp)
file (GLOB H_FILES include/*.h)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
set (INCLUDE_DIRS include ${CMAKE_BINARY_DIR}/${DEST_INCLUDE_DIR}/ThirdParty/ImGui)

# These are used to create visual studio folders.
source_group(src FILES ${CPP_FILES} )
source_group(include FILES ${H_FILES} )

# Setup target
setup_library ()

# Install headers for building and using the Urho3D library
install_header_files (DIRECTORY include/ DESTINATION ${DEST_INCLUDE_DIR}/ThirdParty/ImGui FILES_MATCHING PATTERN *.h BUILD_TREE_ONLY)



