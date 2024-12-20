cmake_minimum_required(VERSION 3.22)
project(society VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")

find_package(SDL2 REQUIRED)
find_package(OpenCV REQUIRED)

set(SOURCE_EXE
    main.c
    app.h           app.c
    hdronly/jquick.h
    hdronly/nuklear_sdl_renderer.h
    hdronly/hdronly.c
    model/icon.h    model/icon.c
    model/gen.h     model/gen.c
    model/rand.h    model/rand.c
    model/serial.h  model/serial.c
    model/world.h   model/world.c
    model/path.h    model/path.c
    model/ai.h      model/ai.c
    view/tileset.h  view/tileset.c
    view/menu.h     view/menu.c
    view/run.c
    view/view.h     view/view.c
)

add_executable(${PROJECT_NAME} ${SOURCE_EXE})

target_include_directories(${PROJECT_NAME} PRIVATE . hdronly model view ${SDL2_INCLUDE_DIRS}})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} -lm)

