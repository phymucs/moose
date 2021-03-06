//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DenseMatrix.h"
#include "ADMaterial.h"

#define usingComputeIncrementalShellStrainMembers                                                  \
  usingMaterialMembers;                                                                            \
  using ADComputeIncrementalShellStrain<compute_stage>::_t_points;                                 \
  using ADComputeIncrementalShellStrain<compute_stage>::_2d_points;                                \
  using ADComputeIncrementalShellStrain<compute_stage>::_nodes;                                    \
  using ADComputeIncrementalShellStrain<compute_stage>::_ge;                                       \
  using ADComputeIncrementalShellStrain<compute_stage>::_ge_old;                                   \
  using ADComputeIncrementalShellStrain<compute_stage>::_J_map;                                    \
  using ADComputeIncrementalShellStrain<compute_stage>::_J_map_old;                                \
  using ADComputeIncrementalShellStrain<compute_stage>::_node_normal;                              \
  using ADComputeIncrementalShellStrain<compute_stage>::_node_normal_old;                          \
  using ADComputeIncrementalShellStrain<compute_stage>::_strain_vector;                            \
  using ADComputeIncrementalShellStrain<compute_stage>::_soln_vector;                              \
  using ADComputeIncrementalShellStrain<compute_stage>::_B;                                        \
  using ADComputeIncrementalShellStrain<compute_stage>::_strain_increment;                         \
  using ADComputeIncrementalShellStrain<compute_stage>::_total_strain;                             \
  using ADComputeIncrementalShellStrain<compute_stage>::_total_strain_old;                         \
  using ADComputeIncrementalShellStrain<compute_stage>::_v1;                                       \
  using ADComputeIncrementalShellStrain<compute_stage>::_v2;                                       \
  using ADComputeIncrementalShellStrain<compute_stage>::_dxyz_dxi;                                 \
  using ADComputeIncrementalShellStrain<compute_stage>::_dxyz_deta;                                \
  using ADComputeIncrementalShellStrain<compute_stage>::_dxyz_dzeta;                               \
  using ADComputeIncrementalShellStrain<compute_stage>::_thickness;                                \
  using ADComputeIncrementalShellStrain<compute_stage>::_dphidxi_map;                              \
  using ADComputeIncrementalShellStrain<compute_stage>::_dphideta_map;                             \
  using ADComputeIncrementalShellStrain<compute_stage>::_phi_map;                                  \
  using ADComputeIncrementalShellStrain<compute_stage>::_g1_a;                                     \
  using ADComputeIncrementalShellStrain<compute_stage>::_g1_c;                                     \
  using ADComputeIncrementalShellStrain<compute_stage>::_g2_b;                                     \
  using ADComputeIncrementalShellStrain<compute_stage>::_g2_d;                                     \
  using ADComputeIncrementalShellStrain<compute_stage>::_soln_disp_index;                          \
  using ADComputeIncrementalShellStrain<compute_stage>::_soln_rot_index;                           \
  using ADComputeIncrementalShellStrain<compute_stage>::_sol_old;                                  \
  using ADComputeIncrementalShellStrain<compute_stage>::computeBMatrix;                            \
  using ADComputeIncrementalShellStrain<compute_stage>::computeSolnVector;                         \
  using ADComputeIncrementalShellStrain<compute_stage>::updateGVectors;                            \
  using ADComputeIncrementalShellStrain<compute_stage>::updatedxyz;                                \
  using ADComputeIncrementalShellStrain<compute_stage>::computeGMatrix

// Forward Declarations
template <ComputeStage>
class ADComputeIncrementalShellStrain;

namespace libMesh
{
class QGauss;
}

template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;
typedef RankTwoTensorTempl<DualReal> DualRankTwoTensor;

declareADValidParams(ADComputeIncrementalShellStrain);

template <ComputeStage compute_stage>
class ADComputeIncrementalShellStrain : public ADMaterial<compute_stage>
{
public:
  static InputParameters validParams();

  ADComputeIncrementalShellStrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeProperties() override;

  /// Computes the 20x1 soln vector and its derivatives for each shell element
  virtual void computeSolnVector();

  /// Computes the B matrix that connects strains to nodal displacements and rotations
  virtual void computeBMatrix();

  /// Computes the node normal at each node
  virtual void computeNodeNormal();

  /// Updates the vectors required for shear locking computation for finite rotations
  virtual void updateGVectors(){};

  /// Updates covariant vectors at each qp for finite rotations
  virtual void updatedxyz(){};

  /// Computes the transformation matrix from natural coordinates to local cartesian coordinates for elasticity tensor transformation
  virtual void computeGMatrix();

  /// Number of coupled rotational variables
  unsigned int _nrot;

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  /// Variable numbers corresponding to the rotational variables
  std::vector<unsigned int> _rot_num;

  /// Variable numbers corresponding to the displacement variables
  std::vector<unsigned int> _disp_num;

  /// Coupled variable for the shell thickness
  const VariableValue & _thickness;

  /// Flag to compute large strains
  const bool _large_strain;

  /// Strain increment in the covariant coordinate system
  std::vector<ADMaterialProperty(RankTwoTensor) *> _strain_increment;

  /// Total strain increment in the covariant coordinate system
  std::vector<ADMaterialProperty(RankTwoTensor) *> _total_strain;

  /// Old total strain increment in the covariant coordinate system
  std::vector<const MaterialProperty<RankTwoTensor> *> _total_strain_old;

  /// Reference to the nonlinear system object
  NonlinearSystemBase & _nonlinear_sys;

  /// Indices of solution vector corresponding to displacement DOFs in 3 directions at the 4 nodes
  std::vector<std::vector<unsigned int>> _soln_disp_index;

  /// Indices of solution vector corresponding to rotation DOFs in 2 directions at the 4 nodes
  std::vector<std::vector<unsigned int>> _soln_rot_index;

  /// Vector that stores the incremental solution at all the 20 DOFs in the 4 noded element.
  ADDenseVector _soln_vector;

  /// Vector that stores the strain in the the 2 axial and 3 shear directions
  ADDenseVector _strain_vector;

  /// Vector storing pointers to the nodes of the shell element
  std::vector<const Node *> _nodes;

  /// Material property storing the normal to the element at the 4 nodes. Stored as a material property for convinience.
  ADMaterialProperty(RealVectorValue) & _node_normal;

  /// Material property storing the old normal to the element at the 4 nodes.
  const MaterialProperty<RealVectorValue> & _node_normal_old;

  /// Quadrature rule in the out of plane direction
  std::unique_ptr<QGauss> _t_qrule;

  /// Quadrature points in the out of plane direction in isoparametric coordinate system
  std::vector<Point> _t_points;

  /// Quadrature points in the in-plane direction in isoparametric coordinate system
  std::vector<Point> _2d_points;

  /// Derivatives of shape functions w.r.t isoparametric coordinates xi
  std::vector<std::vector<Real>> _dphidxi_map;

  /// Derivatives of shape functions w.r.t isoparametric coordinates eta
  std::vector<std::vector<Real>> _dphideta_map;

  /// Shape function value
  std::vector<std::vector<Real>> _phi_map;

  /// Derivative of global x, y and z w.r.t isoparametric coordinate xi
  std::vector<ADMaterialProperty(RealVectorValue) *> _dxyz_dxi;

  /// Derivative of global x, y and z w.r.t isoparametric coordinate eta
  std::vector<ADMaterialProperty(RealVectorValue) *> _dxyz_deta;

  /// Derivative of global x, y and z w.r.t isoparametric coordinate zeta
  std::vector<ADMaterialProperty(RealVectorValue) *> _dxyz_dzeta;

  /// Old derivative of global x, y and z w.r.t isoparametric coordinate xi
  std::vector<const MaterialProperty<RealVectorValue> *> _dxyz_dxi_old;

  /// Old derivative of global x, y and z w.r.t isoparametric coordinate eta
  std::vector<const MaterialProperty<RealVectorValue> *> _dxyz_deta_old;

  /// Old derivative of global x, y and z w.r.t isoparametric coordinate zeta
  std::vector<const MaterialProperty<RealVectorValue> *> _dxyz_dzeta_old;

  /// First tangential vectors at nodes
  std::vector<ADRealVectorValue> _v1;

  /// First tangential vectors at nodes
  std::vector<ADRealVectorValue> _v2;

  /// B_matrix for small strain
  std::vector<ADMaterialProperty(DenseMatrix<Real>) *> _B;

  /// Old B_matrix for small strain
  std::vector<const MaterialProperty<DenseMatrix<Real>> *> _B_old;

  /// ge matrix for elasticity tensor conversion
  std::vector<ADMaterialProperty(RankTwoTensor) *> _ge;

  /// Old ge matrix for elasticity tensor conversion
  std::vector<const MaterialProperty<RankTwoTensor> *> _ge_old;

  /// Material property containing jacobian of transformation
  std::vector<ADMaterialProperty(Real) *> _J_map;

  /// Old material property containing jacobian of transformation
  std::vector<const MaterialProperty<Real> *> _J_map_old;

  /// Rotation matrix material property
  std::vector<MaterialProperty<RankTwoTensor> *> _rotation_matrix;

  /// Total strain in global coordinate system
  std::vector<MaterialProperty<RankTwoTensor> *> _total_global_strain;

  /// simulation variables
  ADRealVectorValue _x2;
  ADRealVectorValue _x3;
  const NumericVector<Number> * const & _sol;
  const NumericVector<Number> & _sol_old;
  ADRealVectorValue _g3_a;
  ADRealVectorValue _g3_c;
  ADRealVectorValue _g3_b;
  ADRealVectorValue _g3_d;
  ADRealVectorValue _g1_a;
  ADRealVectorValue _g1_c;
  ADRealVectorValue _g2_b;
  ADRealVectorValue _g2_d;
  RankTwoTensor _unrotated_total_strain;

  usingMaterialMembers;
};
