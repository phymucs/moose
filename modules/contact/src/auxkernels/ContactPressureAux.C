//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactPressureAux.h"

#include "NodalArea.h"
#include "PenetrationLocator.h"

#include "libmesh/string_to_enum.h"

registerMooseObject("ContactApp", ContactPressureAux);

defineLegacyParams(ContactPressureAux);

InputParameters
ContactPressureAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("nodal_area", "The nodal area");
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONLINEAR;
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");
  params.addParam<MooseEnum>("order", orders, "The finite element order: " + orders.getRawNames());

  params.addClassDescription("Computes the contact pressure from the contact force and nodal area");

  return params;
}

ContactPressureAux::ContactPressureAux(const InputParameters & params)
  : AuxKernel(params),
    _nodal_area(coupledValue("nodal_area")),
    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("paired_boundary"),
                              getParam<std::vector<BoundaryName>>("boundary")[0],
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order"))))
{
}

ContactPressureAux::~ContactPressureAux() {}

Real
ContactPressureAux::computeValue()
{
  Real value(0);
  const Real area = _nodal_area[_qp];
  const PenetrationInfo * pinfo(NULL);

  const auto it = _penetration_locator._penetration_info.find(_current_node->id());
  if (it != _penetration_locator._penetration_info.end())
    pinfo = it->second;

  if (pinfo && area != 0)
    value = -(pinfo->_contact_force * pinfo->_normal) / area;

  return value;
}
