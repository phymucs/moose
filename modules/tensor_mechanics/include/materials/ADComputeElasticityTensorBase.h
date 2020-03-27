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
#include "RankFourTensor.h"
#include "GuaranteeProvider.h"
#include "DerivativeMaterialPropertyNameInterface.h"

#define usingComputeElasticityTensorBaseMembers                                                    \
  usingMaterialMembers;                                                                            \
  using ADComputeElasticityTensorBase::_elasticity_tensor_name;                                    \
  using ADComputeElasticityTensorBase::_elasticity_tensor;                                         \
  using ADComputeElasticityTensorBase::issueGuarantee

class ADComputeElasticityTensorBase;
template <typename>
class RankFourTensorTempl;
typedef RankFourTensorTempl<Real> RankFourTensor;

declareADValidParams(ADComputeElasticityTensorBase);

/**
 * ADComputeElasticityTensorBase is a the base class for computing elasticity tensors
 */
class ADComputeElasticityTensorBase : public ADMaterial,
                                      public DerivativeMaterialPropertyNameInterface,
                                      public GuaranteeProvider
{
public:
  static InputParameters validParams();

  ADComputeElasticityTensorBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void computeQpElasticityTensor() = 0;

  /// Base name of the material system
  const std::string _base_name;

  std::string _elasticity_tensor_name;

  ADMaterialProperty<RankFourTensor> & _elasticity_tensor;

  /// prefactor function to multiply the elasticity tensor with
  const Function * const _prefactor_function;

};
