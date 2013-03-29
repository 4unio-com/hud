add_subdirectory(man)
add_subdirectory(libhud-client)

install(
	FILES
		HUD\ Architecture.svg
		libhud-communication.msc
		libhud-client.msc
	DESTINATION
		${CMAKE_INSTALL_DOCDIR}
)

macro(add_chart CHART_NAME PNG_NAME)
	execute_process(COMMAND mscgen -T png -i ${CMAKE_CURRENT_SOURCE_DIR}/${CHART_NAME} -o ${PNG_NAME})
endmacro()

add_chart(libhud-communcation.msc libhud-communcation.png)
add_chart(libhud-client.msc libhud-client.png)
