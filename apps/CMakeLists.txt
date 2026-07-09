set(MolesimVis_Sources
    MolesimVis.cpp
    Simulation.hpp
    Simulation.cpp
)

add_executable(MolesimVis ${MolesimVis_Sources})
target_link_libraries(MolesimVis chemfiles Agpu ${SDL2_LIBRARY})
