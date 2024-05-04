add_library(accipit SHARED
    ir.cpp
    type.cpp
)
add_library(accsys::ir ALIAS accipit)