add_executable(create-htgfile create-htgfile.cpp)
target_link_libraries(create-htgfile PRIVATE oktal)

add_executable(poisson poisson.cpp)
target_link_libraries(poisson PRIVATE oktal)
