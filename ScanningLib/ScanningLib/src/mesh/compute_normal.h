#include <ctype.h>
#include <iomanip>
#include <list>
// #include <shlobj.h>
// #include <windows.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <vcg/complex/algorithms/local_optimization.h>
#include <vcg/complex/algorithms/local_optimization/tri_edge_collapse_quadric_tex.h>
#include <vcg/complex/algorithms/pointcloud_normal.h>
#include <vcg/complex/allocate.h>
//#include <vcg/simplex/face/component_ocf.h>
#include <vcg/container/simple_temporary_data.h>
#include <vcg/math/quadric5.h>
#include <vcg/space/box3.h>
#include <wrap/io_trimesh/export.h>
#include <wrap/io_trimesh/export_obj.h>
#include <wrap/io_trimesh/export_ply.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/import_ply.h>
#include <wrap/ply/plylib.h>

#include "mesh/mesh.h"

using namespace std;
using namespace vcg;

bool updateMask(CMeshO& cm, int mask);
void updateDataMask(CMeshO& cm, int neededDataMask);
void ComputePCNormal(const std::string& input_name, const std::string& output_name);
void cutFaces(const std::string& input_name, const std::string& output_name,float * plane,bool useMarker);
void smoothMesh(const std::string& input_name, const std::string& output_name,bool binary);
void invertMeshNormal(const std::string& input_name, const std::string& output_name,bool binary);
template <class MeshType>
void PoissonClean(MeshType &m, bool scaleNormal, bool cleanFlag)
{
    vcg::tri::UpdateNormal<MeshType>::NormalizePerVertex(m);
    
    if(cleanFlag) {
        for (auto vi = m.vert.begin(); vi != m.vert.end(); ++vi)
            if (vcg::SquaredNorm(vi->N()) < std::numeric_limits<float>::min()*10.0)
                vcg::tri::Allocator<MeshType>::DeleteVertex(m,*vi);
        
        for (auto fi = m.face.begin(); fi != m.face.end(); ++fi)
            if( fi->V(0)->IsD() || fi->V(1)->IsD() || fi->V(2)->IsD() )
                vcg::tri::Allocator<MeshType>::DeleteFace(m,*fi);
    }
    
    vcg::tri::Allocator<MeshType>::CompactEveryVector(m);
    if(scaleNormal)
    {
        for(auto vi=m.vert.begin();vi!=m.vert.end();++vi)
            vi->N() *= vi->Q();
    }
}