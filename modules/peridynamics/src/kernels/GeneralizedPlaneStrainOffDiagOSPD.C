//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainOffDiagOSPD.h"
#include "MooseVariableScalar.h"
#include "PeridynamicsMesh.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("PeridynamicsApp", GeneralizedPlaneStrainOffDiagOSPD);

defineLegacyParams(GeneralizedPlaneStrainOffDiagOSPD);

InputParameters
GeneralizedPlaneStrainOffDiagOSPD::validParams()
{
  InputParameters params = MechanicsBasePD::validParams();
  params.addClassDescription(
      "Class for calculating the off-diagonal Jacobian corresponding to "
      "coupling between displacements (or temperature) and the scalar out-of-plane strain for "
      "the generalized plane strain using the OSPD formulation");

  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for strain in the out-of-plane direction");

  return params;
}

GeneralizedPlaneStrainOffDiagOSPD::GeneralizedPlaneStrainOffDiagOSPD(
    const InputParameters & parameters)
  : MechanicsBasePD(parameters),
    _bond_local_dfdE(getMaterialProperty<Real>("bond_local_dfdE")),
    _bond_nonlocal_dfdE(getMaterialProperty<Real>("bond_nonlocal_dfdE")),
    _alpha(getMaterialProperty<Real>("thermal_expansion_coeff")),
    _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
    _scalar_out_of_plane_strain_var_num(coupledScalar("scalar_out_of_plane_strain"))
{
  // Consistency check
  if (_disp_var.size() != 2)
    mooseError("GeneralizedPlaneStrain only works for two dimensional case!");
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeOffDiagJacobianScalar(unsigned int jvar_num)
{
  if (jvar_num == _scalar_out_of_plane_strain_var_num)
  {
    prepare();

    if (_var.number() == _disp_var[0]->number())
      if (_use_full_jacobian)
        computeDispFullOffDiagJacobianScalar(0, jvar_num);
      else
        computeDispPartialOffDiagJacobianScalar(0, jvar_num);
    else if (_var.number() == _disp_var[1]->number())
      if (_use_full_jacobian)
        computeDispFullOffDiagJacobianScalar(1, jvar_num);
      else
        computeDispPartialOffDiagJacobianScalar(1, jvar_num);
    else if (_temp_coupled ? _var.number() == _temp_var->number() : 0)
      computeTempOffDiagJacobianScalar(jvar_num);
  }
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeDispFullOffDiagJacobianScalar(unsigned int component,
                                                                        unsigned int jvar_num)
{

  DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar_num);
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar_num, _var.number());
  MooseVariableScalar & jvar = _sys.getScalarVariable(_tid, jvar_num);

  // LOCAL jacobian contribution
  // fill in the column corresponding to the scalar variable from bond ij
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jvar.order(); _j++)
      ken(_i, _j) +=
          (_i == _j ? -1 : 1) * _current_unit_vec(component) * _bond_local_dfdE[0] * _bond_status;

  // NONLOCAL jacobian contribution
  std::vector<RankTwoTensor> shape(_nnodes), dgrad(_nnodes);

  // for off-diagonal low components
  RankTwoTensor delta(RankTwoTensor::initIdentity);

  for (unsigned int nd = 0; nd < _nnodes; nd++)
  {
    if (_dim == 2)
      shape[nd](2, 2) = dgrad[nd](2, 2) = 1.0;
    // calculation of jacobian contribution to current node's neighbors
    std::vector<dof_id_type> ivardofs(_nnodes);
    ivardofs[nd] = _ivardofs[nd];
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

    Real vol_nb, weight;
    RealGradient origin_vec_nb, current_vec_nb;

    for (unsigned int nb = 0; nb < neighbors.size(); nb++)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
      {
        Node * neighbor_nb = _pdmesh.nodePtr(neighbors[nb]);
        ivardofs[1 - nd] = neighbor_nb->dof_number(_sys.number(), _var.number(), 0);
        vol_nb = _pdmesh.getPDNodeVolume(neighbors[nb]);

        // obtain bond nb's origin length and current orientation
        origin_vec_nb = *neighbor_nb - *_pdmesh.nodePtr(_current_elem->node_id(nd));

        for (unsigned int k = 0; k < _dim; k++)
          current_vec_nb(k) = origin_vec_nb(k) + _disp_var[k]->getNodalValue(*neighbor_nb) -
                              _disp_var[k]->getNodalValue(*_current_elem->node_ptr(nd));

        weight = _horiz_rad[nd] / origin_vec_nb.norm();
        // prepare shape tensor and deformation gradient tensor for current node
        for (unsigned int k = 0; k < _dim; k++)
          for (unsigned int l = 0; l < _dim; l++)
          {
            shape[nd](k, l) += weight * origin_vec_nb(k) * origin_vec_nb(l) * vol_nb;
            dgrad[nd](k, l) += weight * current_vec_nb(k) * origin_vec_nb(l) * vol_nb;
          }

        // cache the nonlocal jacobian contribution
        _local_ke.resize(ken.m(), ken.n());
        _local_ke.zero();
        _local_ke(0, 0) = (nd == 0 ? -1 : 1) * current_vec_nb(component) / current_vec_nb.norm() *
                          _bond_nonlocal_dfdE[nd] / origin_vec_nb.norm() * vol_nb * _bond_status;
        _local_ke(1, 0) = (nd == 0 ? 1 : -1) * current_vec_nb(component) / current_vec_nb.norm() *
                          _bond_nonlocal_dfdE[nd] / origin_vec_nb.norm() * vol_nb * _bond_status;

        _assembly.cacheJacobianBlock(_local_ke, ivardofs, jvar.dofIndices(), _var.scalingFactor());
      }

    // finalize the shape tensor and deformation gradient tensor for node_i
    if (MooseUtils::absoluteFuzzyEqual(shape[nd].det(), 0.0))
    {
      shape[nd] = delta;
      dgrad[nd] = delta;
    }
    else
    {
      // inverse the shape tensor at node i
      shape[nd] = shape[nd].inverse();
      // calculate the deformation gradient tensor at node i
      dgrad[nd] = dgrad[nd] * shape[nd];
    }
  }

  // off-diagonal jacobian entries on the row
  Real dEidUi =
      -_node_vol[1] * _horiz_rad[0] / _origin_vec.norm() *
      (_Cijkl[0](2, 2, 0, 0) * (_origin_vec(0) * shape[0](0, 0) + _origin_vec(1) * shape[0](1, 0)) *
           dgrad[0](component, 0) +
       _Cijkl[0](2, 2, 1, 1) * (_origin_vec(0) * shape[0](0, 1) + _origin_vec(1) * shape[0](1, 1)) *
           dgrad[0](component, 1));
  Real dEjdUj =
      _node_vol[0] * _horiz_rad[1] / _origin_vec.norm() *
      (_Cijkl[0](2, 2, 0, 0) * (_origin_vec(0) * shape[1](0, 0) + _origin_vec(1) * shape[1](1, 0)) *
           dgrad[1](component, 0) +
       _Cijkl[0](2, 2, 1, 1) * (_origin_vec(0) * shape[1](0, 1) + _origin_vec(1) * shape[1](1, 1)) *
           dgrad[1](component, 1));

  // fill in the row corresponding to the scalar variable
  kne(0, 0) += (dEidUi * _node_vol[0] - dEjdUj * _node_vol[1]) * _bond_status; // node i
  kne(0, 1) += (dEjdUj * _node_vol[1] - dEidUi * _node_vol[0]) * _bond_status; // node j
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeDispPartialOffDiagJacobianScalar(unsigned int component,
                                                                           unsigned int jvar_num)
{
  DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar_num);
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar_num, _var.number());
  MooseVariableScalar & jvar = _sys.getScalarVariable(_tid, jvar_num);

  // fill in the column corresponding to the scalar variable from bond ij
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jvar.order(); _j++)
    {
      ken(_i, _j) +=
          (_i == _j ? -1 : 1) * _current_unit_vec(component) * _bond_local_dfdE[0] * _bond_status;
      kne(_j, _i) +=
          (_i == _j ? -1 : 1) * _current_unit_vec(component) * _bond_local_dfdE[0] * _bond_status;
    }
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeTempOffDiagJacobianScalar(unsigned int jvar_num)
{
  // off-diagonal jacobian entries on the row

  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar_num, _var.number());

  // number of neighbors for node i and j
  unsigned int nn_i = _pdmesh.getNeighbors(_current_elem->node_id(0)).size();
  unsigned int nn_j = _pdmesh.getNeighbors(_current_elem->node_id(1)).size();

  //  one-way coupling between the strain_zz and temperature. fill in the row corresponding to the
  //  scalar variable strain_zz
  kne(0, 0) += -_alpha[0] *
               (_Cijkl[0](2, 2, 0, 0) + _Cijkl[0](2, 2, 1, 1) + _Cijkl[0](2, 2, 2, 2)) *
               _node_vol[0] / nn_i; // node i
  kne(0, 1) += -_alpha[0] *
               (_Cijkl[0](2, 2, 0, 0) + _Cijkl[0](2, 2, 1, 1) + _Cijkl[0](2, 2, 2, 2)) *
               _node_vol[1] / nn_j; // node j
}
