cmake_minimum_required(VERSION 3.10)
project(HAPP)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(happ
    src/main.cpp
    src/hardware/storage_type.cpp
    src/hardware/hardware_config.cpp
    src/workload/workload_spec.cpp
    src/simulator/simulation_engine.cpp
    src/simulator/simulation_result.cpp
    src/profiler/bottleneck_analyzer.cpp
    src/cli/cli_parser.cpp
    src/cli/cli_runner.cpp
    src/cli/constrained_session.cpp
    src/utils/config_file.cpp
    src/utils/virtual_memory.cpp
)

if(WIN32)
    target_compile_definitions(happ PRIVATE _WIN32_WINNT=0x0A00)
endif()
