//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

#define usingComputeStrainBaseMembers                                                              \
  usingMaterialMembers;                                                                            \
  using ADComputeStrainBase::_ndisp;                                                \
  using ADComputeStrainBase::_disp;                                                 \
  using ADComputeStrainBase::_grad_disp;                                            \
  using ADComputeStrainBase::_base_name;                                            \
  using ADComputeStrainBase::_mechanical_strain;                                    \
  using ADComputeStrainBase::_global_strain;                                        \
  using ADComputeStrainBase::_volumetric_locking_correction;                        \
  using ADComputeStrainBase::_current_elem_volume;                                  \
  using ADComputeStrainBase::_eigenstrain_names;                                    \
  using ADComputeStrainBase::_eigenstrains;                                         \
  using ADComputeStrainBase::_total_strain

// Forward Declarations
class ADComputeStrainBase;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;
typedef RankTwoTensorTempl<DualReal> DualRankTwoTensor;

declareADValidParams(ADComputeStrainBase);

/**
 * ADADComputeStrainBase is the base class for strain tensors
 */
class ADComputeStrainBase : public ADMaterial
{
public:
  static InputParameters validParams();

  ADComputeStrainBase(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void displacementIntegrityCheck();

  /// Coupled displacement variables
  const unsigned int _ndisp;

  /// Displacement variables
  std::vector<const ADVariableValue *> _disp;

  /// Gradient of displacements
  std::vector<const ADVariableGradient *> _grad_disp;

  /// Base name of the material system
  const std::string _base_name;

  ADMaterialProperty<RankTwoTensor> & _mechanical_strain;
  ADMaterialProperty<RankTwoTensor> & _total_strain;

  std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const ADMaterialProperty<RankTwoTensor> *> _eigenstrains;

  const ADMaterialProperty<RankTwoTensor> * _global_strain;

  const bool _volumetric_locking_correction;
  const Real & _current_elem_volume;

};
