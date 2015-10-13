
cmake_minimum_required(VERSION 2.8)

PROJECT(ContourWidget)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(ContourWidget MACOSX_BUNDLE ContourWidget)

if(VTK_LIBRARIES)
  target_link_libraries(ContourWidget ${VTK_LIBRARIES})
else()
  target_link_libraries(ContourWidget vtkHybrid vtkWidgets)
endif()
