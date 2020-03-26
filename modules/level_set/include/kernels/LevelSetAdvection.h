//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ADKernelValue.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
class LevelSetAdvection;

declareADValidParams(LevelSetAdvection);

/**
 * Advection Kernel for the levelset equation.
 *
 * \psi_i \vec{v} \nabla u,
 * where \vec{v} is the interface velocity that is a set of
 * coupled variables.
 */
class LevelSetAdvection : public LevelSetVelocityInterface<ADKernelValue>
{
public:
  static InputParameters validParams();

  LevelSetAdvection(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  using LevelSetVelocityInterface<ADKernelValue>::computeQpVelocity;
  using LevelSetVelocityInterface<ADKernelValue>::_velocity;
};

