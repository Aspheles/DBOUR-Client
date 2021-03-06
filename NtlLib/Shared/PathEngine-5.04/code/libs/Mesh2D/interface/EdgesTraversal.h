//**********************************************************************
//
// Copyright (c) 2004
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

#ifndef EDGES_TRAVERSAL_INCLUDED
#define EDGES_TRAVERSAL_INCLUDED

#include "libs/Mesh2D/interface/tMesh.h"
#include "common/STL/vector.h"

template <class T> class cElementEdge;

class cEdgesTraversal
{
    std::vector<tEdge> _edges;

    // prevent copying and assignment
    cEdgesTraversal(const cEdgesTraversal&);
    const cEdgesTraversal& operator=(const cEdgesTraversal&);

public:

    cEdgesTraversal() {}
    cEdgesTraversal(tFace startFace);
    cEdgesTraversal(tEdge entryEdge);

    void init_InFace(tFace startFace);
    void init_Entry(tEdge entryEdge);
    void init_AsClone(const cEdgesTraversal& rhs)
    {
        _edges = rhs._edges;
    }

    void swap(cEdgesTraversal& rhs) {_edges.swap(rhs._edges);}

    void reverse();
    void clear() {_edges.clear();}

    bool isValid() const;
    bool empty() const {return _edges.empty();}

    void pushEdgeCrossing(tEdge crossed);
    void pushExit(tEdge exitEdge);
    void popEdgeCrossing();
    void popExit();

    long facesCrossed() const
    {
        long result = SizeL(_edges);
        if(enters())
        {
            result--;
        }
        if(exits())
        {
            result--;
        }
        return result;
    }
    tFace getFaceCrossed(long index) const; //.. lose this one
    tFace faceCrossed(long index) const
    {
        return getFaceCrossed(index);
    }
    tFace getLastFaceCrossed() const
    {
        return getFaceCrossed(facesCrossed() - 1);
    }

    long indexForFace(tFace f) const;

	long getEdgesCrossed() const;
	tEdge getEdgeCrossed(long index) const;

    tEdge getEdgeCrossed_DoesntEnter(long index) const
    {
        assertD(!enters());
    	return _edges[index + 1].twin();
    }

	tEdge getLastEdgeCrossed() const;

    bool enters() const;
    bool exits() const;

    tFace getExternalFaceForEntry() const;
    tFace getExternalFaceForExit() const;

    tEdge entryEdge() const
    {
        assertD(enters());
        return _edges[1];
    }
    tEdge exitEdge() const
    {
        assertD(exits());
        return _edges.back().twin();
    }
};

#endif
