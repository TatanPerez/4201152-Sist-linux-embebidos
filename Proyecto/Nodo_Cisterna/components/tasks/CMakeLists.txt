# CMakeLists.txt para componente Tasks

idf_component_register(SRCS "tasks.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver freertos)
