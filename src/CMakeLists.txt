cmake_minimum_required(VERSION 3.22)
project(society VERSION 0.0.1 LANGUAGES C)

find_package(SDL2 REQUIRED)
find_package(OpenCV REQUIRED)

set(SOURCE_EXE
    main.c
    app.h app.c
    hdronly/nuklear_sdl_renderer.h
    hdronly/hdronly.c
    model/gen.h model/gen.c
    model/rand.h model/rand.c
    model/serial.h model/serial.c
    model/world.h model/world.c
    view/menu.h view/menu.c
    view/run.c
    view/view.h view/view.c
)

add_executable(${PROJECT_NAME} ${SOURCE_EXE})

target_include_directories(${PROJECT_NAME} PRIVATE . hdronly model view ${SDL2_INCLUDE_DIRS}})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} -lm)

