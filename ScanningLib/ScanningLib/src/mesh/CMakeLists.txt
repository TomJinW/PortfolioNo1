set(FOLDER_NAME "mesh")

# VCG Bug: need to add plylib to library

add_library(mesh
  compute_normal.h compute_normal.cc 
  plylib.h plylib.cc
)