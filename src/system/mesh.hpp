/* *************************************************************
 *  
 *   Soft Active Mater on Surfaces (SAMoS)
 *   
 *   Author: Rastko Sknepnek
 *  
 *   Division of Physics
 *   School of Engineering, Physics and Mathematics
 *   University of Dundee
 *   
 *   (c) 2013, 2014
 * 
 *   School of Science and Engineering
 *   School of Life Sciences 
 *   University of Dundee
 * 
 *   (c) 2015
 * 
 *   Author: Silke Henkes
 * 
 *   Department of Physics 
 *   Institute for Complex Systems and Mathematical Biology
 *   University of Aberdeen  
 * 
 *   (c) 2014, 2015
 *  
 *   This program cannot be used, copied, or modified without
 *   explicit written permission of the authors.
 * 
 * ************************************************************* */

/*!
 * \file mesh.hpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 18-Nov-2015
 * \brief Declaration of Mesh class.
 */ 

#ifndef __MESH_HPP__
#define __MESH_HPP__

#include "vertex.hpp"
#include "edge.hpp"
#include "face.hpp"

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <algorithm>
#include <exception>

#include <boost/format.hpp>

using boost::format;
using std::string;
using std::vector;
using std::map;
using std::pair;
using std::make_pair;
using std::endl;
using std::copy;
using std::reverse;
using std::runtime_error;

typedef pair<int,int> VertexPair;

/*! Mesh class handles basic manipuations with mesesh
 *
 */
class Mesh
{
public:
  //! Construct a Mesh object
  Mesh() : m_size(0), m_nedge(0), m_nface(0) {   }
  
  //! Get mesh size
  int size() { return m_size; }
  
  //! Get number of edges 
  int nedges() { return m_nedge; }
  
  //! Get number of faces
  int nfaces() { return m_nface; }
  
  //! Get list of all vertices
  vector<Vertex>& get_vertices() { return m_vertices; }
  
  //! Get edge list 
  vector<Edge>& get_edges() { return m_edges; }
  
  //! Get list of faces
  vector<Face>& get_faces() { return m_faces; }
  
  //! Get edge-face data structure
  map<pair<int,int>, int>& get_edge_face() { return m_edge_face; }
  
  //! Resets the mesh
  void reset();
  
  //! Add a vertex
  //! \param p particle
  //add_vertex(Particle& p)
  //{
  //  vertices.push_back(Vertex(p));
  //  size++;
  //}
  
  //! Add a vertex
  //! \param vid vertex id
  //! \param x x-coordinate
  //! \param y y-coordinate
  //! \param z z-coordinate
  void add_vertex(int vid, double x, double y, double z)
  {
    m_vertices.push_back(Vertex(vid,x,y,z));
    m_size++;
  }
  
  //! Add from particle 
  //! \param p particle
  void add_vertex(Particle& p)
  {
    m_vertices.push_back(Vertex(p));
    m_size++;
  }
  
  //! Add an edge
  void add_edge(int,int);
    
  //! Add face
  void add_face(vector<int>&);
  
  //! Updates vertex positions 
  //! \param p particle
  void update(Particle& p)
  {
    m_vertices[p.get_id()].r = Vector3d(p.x, p.y, p.z);
    m_vertices[p.get_id()].type = p.get_type();
  }
  
  //! Post-processes the mesh
  void postprocess();
  
  //! Compute face centre
  void compute_centre(int);
  
  //! Order vertices in a face
  void order_face(int);
  
  //! Order vertex star
  void order_star(int);
  
  //! Compute dual area
  double dual_area(int,Vector3d&);
  
  //! Dual perimeter
  double dual_perimeter(int);
  

private:  
  
  int m_size;    //!< Mesh size
  int m_nedge;   //!< Number of edges
  int m_nface;   //!< Number of faces
    
  vector<Vertex> m_vertices;           //!< Contains all vertices
  vector<Edge> m_edges;                //!< Contains all edge
  vector<Face> m_faces;                //!< Contains all faces
  map<pair<int,int>, int> m_edge_map;  //!< Relates vertex indices to edge ids
  map<pair<int,int>, int> m_edge_face; //!< Relates pairs of faces to edges
  
};

#endif