if(NOT DEFINED PROGRAM OR NOT DEFINED MODE OR NOT DEFINED EXPECTED_CODE)
	message(FATAL_ERROR "PROGRAM, MODE and EXPECTED_CODE are required")
endif()
execute_process(
	COMMAND "${PROGRAM}" "${MODE}"
	RESULT_VARIABLE ExitCode
	OUTPUT_VARIABLE StandardOutput
	ERROR_VARIABLE StandardError
	TIMEOUT 20
)
if(NOT "${ExitCode}" STREQUAL "${EXPECTED_CODE}")
	message(FATAL_ERROR
		"Unexpected exit for ${MODE}:got '${ExitCode}', expected '${EXPECTED_CODE}'\n${StandardOutput}\n${StandardError}")
endif()