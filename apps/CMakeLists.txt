set(Molesim_Sources
    Molesim.cpp
)

add_executable(Molesim ${Molesim_Sources})
target_link_libraries(Molesim chemfiles)
