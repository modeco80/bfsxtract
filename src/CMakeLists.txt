add_executable(bfsxtract
		main.cpp
		)

target_link_libraries(bfsxtract PRIVATE
	elfio::elfio)


set_target_properties(bfsxtract PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
		)
