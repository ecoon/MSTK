#include <UnitTest++.h>

#include "../../include/MSTK.h"

// Test if we can split an edge correctly in a 2D and 3D mesh

TEST(ME_Split) 
{
  int idx, i, gid, gdim, *fedir, nef, ok, nve;
  Mesh_ptr mesh;
  MVertex_ptr v0, v1, vnew;
  MEdge_ptr me, enew[2];
  MFace_ptr ef;
  List_ptr efaces, fverts, velist;
  double xyz0[3], xyz1[3], xyznew[3];

  MSTK_Init();

  mesh = MESH_New(UNKNOWN_REP);
  ok = MESH_InitFromFile(mesh,"serial/reghex3D.mstk",NULL);
  CHECK_EQUAL(ok,1);

  CHECK(MESH_Num_Vertices(mesh) > 0);
  
  idx = 0;
  while ((me = MESH_Next_Edge(mesh,&idx))) {
    if (ME_GEntDim(me) == 3) {
      
      /* Get vertices of edge */

      v0 = ME_Vertex(me,0);
      MV_Coords(v0, xyz0);

      v1 = ME_Vertex(me,1);
      MV_Coords(v1, xyz1);

      /* Get its classification info */

      gid = ME_GEntID(me);
      gdim = ME_GEntDim(me);

      /* Get faces connected to edge and the direction in which
       they use the edge*/
      
      efaces = ME_Faces(me);
      nef = List_Num_Entries(efaces);
      fedir = (int *) malloc(nef*sizeof(efaces));
      for (int j = 0; j < nef; j++) {
        ef = List_Entry(efaces,j);
        fedir[j] = MF_EdgeDir(ef,me);
      }

      /* Create new vertex */

      for (int j = 0; j < 3; j++) 
        xyznew[j] = (xyz0[j] + xyz1[j])/2.0;

      /* split the edge */

      vnew = ME_Split(me,xyznew);
      CHECK(vnew);

      velist = MV_Edges(vnew);
      nve = List_Num_Entries(velist);
      CHECK_EQUAL(2,nve);

      enew[0] = List_Entry(velist,0);
      enew[1] = List_Entry(velist,1);

      List_Delete(velist);

      /* check the resulting edges */

      CHECK_EQUAL(v0,ME_Vertex(enew[0],0));
      CHECK_EQUAL(vnew,ME_Vertex(enew[0],1));
      CHECK_EQUAL(vnew,ME_Vertex(enew[1],0));
      CHECK_EQUAL(v1,ME_Vertex(enew[1],1));
      CHECK_EQUAL(gdim,ME_GEntDim(enew[0]));
      CHECK_EQUAL(gid,ME_GEntID(enew[0]));
      CHECK_EQUAL(gdim,ME_GEntDim(enew[1]));
      CHECK_EQUAL(gid,ME_GEntID(enew[1]));

      /* check the resulting faces */

      for (int j = 0; j < nef; j++) {
        ef = List_Entry(efaces,j);
        CHECK_EQUAL(MF_EdgeDir(ef,enew[0]),fedir[j]);
        fverts = MF_Vertices(ef,fedir[j],v0);
        CHECK_EQUAL(List_Entry(fverts,0),v0);
        CHECK_EQUAL(List_Entry(fverts,1),vnew);
        CHECK_EQUAL(List_Entry(fverts,2),v1);
        List_Delete(fverts);
      }

      free(fedir);
      List_Delete(efaces);

      break;
    }
  }
}
