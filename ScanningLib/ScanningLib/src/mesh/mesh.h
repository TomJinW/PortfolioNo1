#ifndef PMVS3_MESH_H
#define PMVS3_MESH_H

#include <vcg/complex/complex.h>

#include <vcg/simplex/face/topology.h>

#include <vcg/complex/algorithms/update/bounding.h>
#include <vcg/complex/algorithms/update/color.h>
#include <vcg/complex/algorithms/update/flag.h>
#include <vcg/complex/algorithms/update/normal.h>
#include <vcg/complex/algorithms/update/position.h>
#include <vcg/complex/algorithms/update/quality.h>
#include <vcg/complex/algorithms/update/selection.h>
#include <vcg/complex/algorithms/update/topology.h>

class CVertexO;
class CEdgeO;
class CFaceO;

class CUsedTypesO : public vcg::UsedTypes<vcg::Use<CVertexO>::AsVertexType,
                                          vcg::Use<CEdgeO>::AsEdgeType,
                                          vcg::Use<CFaceO>::AsFaceType> {};

class CVertexO : public vcg::Vertex<CUsedTypesO, vcg::vertex::InfoOcf, /*  4b */
                                    vcg::vertex::Coord3f,              /* 12b */
                                    vcg::vertex::BitFlags,             /*  4b */
                                    vcg::vertex::Normal3f,             /* 12b */
                                    vcg::vertex::Qualityf,             /*  4b */
                                    vcg::vertex::Color4b,              /*  4b */
                                    vcg::vertex::VFAdjOcf,             /*  0b */
                                    vcg::vertex::MarkOcf,              /*  0b */
                                    vcg::vertex::TexCoordfOcf,         /*  0b */
                                    vcg::vertex::CurvaturefOcf,        /*  0b */
                                    vcg::vertex::CurvatureDirfOcf,     /*  0b */
                                    vcg::vertex::RadiusfOcf            /*  0b */
                                    > {};

// The Main Edge Class
// Currently it does not contains anything.
class CEdgeO
    : public vcg::Edge<CUsedTypesO, vcg::edge::BitFlags, /*  4b */
                       vcg::edge::EVAdj, vcg::edge::EEAdj, vcg::edge::EFAdj> {};

// Each face needs 32 byte, on 32bit arch. and 48 byte on 64bit arch.
class CFaceO : public vcg::Face<CUsedTypesO, vcg::face::InfoOcf, /* 4b */
                                vcg::face::VertexRef,            /*12b */
                                vcg::face::BitFlags,             /* 4b */
                                vcg::face::Normal3f,             /*12b */
                                vcg::face::QualityfOcf,          /* 0b */
                                vcg::face::MarkOcf,              /* 0b */
                                vcg::face::Color4bOcf,           /* 0b */
                                vcg::face::FFAdjOcf,             /* 0b */
                                vcg::face::VFAdjOcf,             /* 0b */
                                vcg::face::FEAdj,                /* 0b */
                                vcg::face::WedgeTexCoordfOcf     /* 0b */
                                > {};

class CMeshO : public vcg::tri::TriMesh<vcg::vertex::vector_ocf<CVertexO>,
                                        vcg::face::vector_ocf<CFaceO> > {
 public:
  int sfn;            // The number of selected faces.
  int svn;            // The number of selected vertices.
  vcg::Matrix44f Tr;  // Usually it is the identity. It is applied in rendering
                      // and filters can or cannot use it. (most of the filter
                      // will ignore this)

  const vcg::Box3f &trBB() {
    static vcg::Box3f bb;
    bb.SetNull();
    bb.Add(Tr, bbox);
    return bb;
  }
};

#endif  // PMVS3_MESH_H