set(MolesimVis_Sources
    MolesimVis.cpp
    Simulation.hpp
    Simulation.cpp
)

add_executable(MolesimVis ${MolesimVis_Sources})
target_link_libraries(MolesimVis chemfiles Agpu ${SDL2_LIBRARY})

set(Molesim_Sources
    Molesim.cpp
    Simulation.hpp
    Simulation.cpp
)

add_executable(Molesim ${Molesim_Sources})
target_link_libraries(Molesim chemfiles Agpu)

set(MolesimBenchmark_Sources
    MolesimBenchmark.cpp
    Simulation.hpp
    Simulation.cpp
)

add_executable(MolesimBenchmark ${MolesimBenchmark_Sources})
target_link_libraries(MolesimBenchmark chemfiles Agpu)
