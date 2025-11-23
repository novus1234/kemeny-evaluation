cmake_minimum_required( VERSION 2.6 FATAL_ERROR )
project(kconsens)

set( Boost_USE_STATIC_LIBS   ON )
set( Boost_USE_MULTITHREADED ON )

set( CMAKE_EXE_LINKER_FLAGS -pthread )

OPTION( BUILD_SHARED_LIBS OFF )

find_package( Boost REQUIRED thread system filesystem )

link_directories ( ${Boost_LIBRARY_DIRS} )

if( Boost_FOUND )
  include_directories(
    ${Boost_INCLUDE_DIRS}
    )
  add_executable(
    kconsens
    main.cpp
    votesmanagement.cpp
    kconsens_instance.cpp
    votetyps.cpp
    kemtyps.cpp
    tools.cpp
    kconsens_instance_bycan.cpp
    kconsens_instance_bycand.cpp
    kconsens_instance_candidate_relations_based.cpp
    kconsens_instance_dirtypairs_based.cpp
    kconsens_instance_bykemX.cpp
    kconsens_instance_approx_pickpermrand.cpp
    kconsens_instance_glpk.cpp
    kconsens_instance_cplex.cpp
    kconsens_instance_gurobi.cpp
    kconsens_instance_charon_hudry.cpp
    randomtools.cpp
    permutationmanagement.cpp
    votesplitter.cpp
    jobmanagement.cpp
    )
  target_link_libraries(
    kconsens
    pthread
    ${Boost_LIBRARIES}
    )
  #set_target_properties(
    #kconsens PROPERTIES
    #COMPILE_FLAGS "${${PROJECT_NAME}_CFLAGS_OTHER}"
    #LINK_FLAGS "-static-libgcc -Wl,-Bstatic ${${PROJECT_NAME}_LDFLAGS_OTHER}"
    #LINK_SEARCH_END_STATIC On
  #)
endif()
