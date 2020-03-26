//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

#define usingSplitCHBaseMembers                                                                    \
  usingKernelMembers;                                                                              \
  using ADSplitCHBase::computeQpResidual;                                           \
  using ADSplitCHBase::computeDFDC

// Forward Declarations
class ADSplitCHBase;

declareADValidParams(ADSplitCHBase);

/**
 * The pair, ADSplitCHCRes and ADSplitCHWRes, splits the Cahn-Hilliard equation
 * by replacing chemical potential with 'w'.
 */
class ADSplitCHBase : public ADKernel
{
public:
  static InputParameters validParams();

  ADSplitCHBase(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();
  virtual ADReal computeDFDC();

};

