/**
 * \file
 * \copyright
 * Copyright (c) 2012-2021, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 */

#include "PhaseTransitionEvaporation.h"

#include "MaterialLib/PhysicalConstant.h"

namespace
{
int numberOfGasComponents(
    std::map<int, std::shared_ptr<MaterialPropertyLib::Medium>> const& media)
{
    // It is always the first (begin) medium that holds fluid phases.
    auto const medium = media.begin()->second;
    auto const& gas_phase = medium->phase("Gas");

    int const n_components_gas = gas_phase.numberOfComponents();

    if (n_components_gas > 2)
    {
        OGS_FATAL(
            "More than two gas phase components are defined. Gaseous mixtures "
            "of more than two components are currently not provided.");
    }
    if (n_components_gas < 2)
    {
        OGS_FATAL(
            "MPL::PhaseTransitionEvaporation() requires at least two "
            "components in the gas phase.");
    }

    return n_components_gas;
}

int findVapourComponentIndex(
    std::map<int, std::shared_ptr<MaterialPropertyLib::Medium>> const& media)
{
    // It is always the first (begin) medium that holds fluid phases.
    auto const medium = media.begin()->second;
    auto const& gas_phase = medium->phase("Gas");
    int const n_components_gas = gas_phase.numberOfComponents();

    // find the component for which the property 'vapour pressure' is defined,
    // using it as the evaporating component.
    for (int c = 0; c < n_components_gas; c++)
    {
        if (gas_phase.component(c).hasProperty(
                MaterialPropertyLib::PropertyType::vapour_pressure))
        {
            return c;
        }
    }

    // A lot of checks can (and should) be done to make sure that the right
    // components with the right properties are used. For example, the names of
    // the components can be compared to check that the name of the evaporable
    // component does not also correspond to the name of the solvate.

    OGS_FATAL(
        "MPL::PhaseTransitionEvaporation(); none of the gas phase components "
        "has a required property vapour_pressure.");
}
}  // namespace

namespace ProcessLib
{
namespace TH2M
{
PhaseTransitionEvaporation::PhaseTransitionEvaporation(
    std::map<int, std::shared_ptr<MaterialPropertyLib::Medium>> const& media)
    : PhaseTransitionModels(media),
      n_components_gas_{numberOfGasComponents(media)},
      gas_phase_vapour_component_index_{findVapourComponentIndex(media)},
      // dry air component is complement of vapour component index
      gas_phase_dry_air_component_index_{gas_phase_vapour_component_index_ ^ 1}
{
    DBUG("Create PhaseTransitionEvaporation constitutive model.");

    // It is always the first (begin) medium that holds fluid phases.
    auto const medium = media.begin()->second;
    auto const& gas_phase = medium->phase("Gas");
    auto const& liquid_phase = medium->phase("AqueousLiquid");

    // check for minimum requirement definitions in media object
    std::array const required_vapour_component_properties = {
        MaterialPropertyLib::specific_latent_heat,
        MaterialPropertyLib::vapour_pressure, MaterialPropertyLib::molar_mass,
        MaterialPropertyLib::specific_heat_capacity,
        MaterialPropertyLib::diffusion};
    std::array const required_dry_air_component_properties = {
        MaterialPropertyLib::molar_mass,
        MaterialPropertyLib::specific_heat_capacity};
    std::array const required_liquid_properties = {
        MaterialPropertyLib::specific_heat_capacity};

    checkRequiredProperties(
        gas_phase.component(gas_phase_vapour_component_index_),
        required_vapour_component_properties);
    checkRequiredProperties(
        gas_phase.component(gas_phase_dry_air_component_index_),
        required_dry_air_component_properties);
    checkRequiredProperties(liquid_phase, required_liquid_properties);
}

void PhaseTransitionEvaporation::getConstitutiveVariables(
    const MaterialPropertyLib::Medium* medium,
    MaterialPropertyLib::VariableArray variables,
    ParameterLib::SpatialPosition pos, double const t, const double dt)
{
    // primary variables
    auto const pGR = std::get<double>(variables[static_cast<int>(
        MaterialPropertyLib::Variable::phase_pressure)]);
    auto const pCap = std::get<double>(variables[static_cast<int>(
        MaterialPropertyLib::Variable::capillary_pressure)]);
    auto const T = std::get<double>(variables[static_cast<int>(
        MaterialPropertyLib::Variable::temperature)]);

    auto const& liquid_phase = medium->phase("AqueousLiquid");
    auto const& gas_phase = medium->phase("Gas");

    constexpr double R = MaterialLib::PhysicalConstant::IdealGasConstant;

    auto const& vapour_component =
        gas_phase.component(gas_phase_vapour_component_index_);
    auto const& dry_air_component =
        gas_phase.component(gas_phase_dry_air_component_index_);

    // specific latent heat (of evaporation)
    const auto dh_evap =
        vapour_component
            .property(MaterialPropertyLib::PropertyType::specific_latent_heat)
            .template value<double>(variables, pos, t, dt);

    variables[static_cast<int>(
        MaterialPropertyLib::Variable::enthalpy_of_evaporation)] = dh_evap;

    // vapour pressure over flat interface
    const auto p_vap_flat =
        vapour_component
            .property(MaterialPropertyLib::PropertyType::vapour_pressure)
            .template value<double>(variables, pos, t, dt);

    const auto dp_vap_flat_dT =
        vapour_component
            .property(MaterialPropertyLib::PropertyType::vapour_pressure)
            .template dValue<double>(variables,
                                     MaterialPropertyLib::Variable::temperature,
                                     pos, t, dt);

    // molar mass of evaporating component (should be the same as
    // solventComponent.molar_mass!)
    auto const M_W =
        vapour_component.property(MaterialPropertyLib::PropertyType::molar_mass)
            .template value<double>(variables, pos, t, dt);
    // molar mass of dry air component (should be the same as
    // solvateComponent.molar_mass!)
    auto const M_C =
        dry_air_component
            .property(MaterialPropertyLib::PropertyType::molar_mass)
            .template value<double>(variables, pos, t, dt);

    rhoLR = liquid_phase.property(MaterialPropertyLib::PropertyType::density)
                .template value<double>(variables, pos, t, dt);
    rhoWLR = rhoLR;

    // Kelvin-Laplace correction for menisci
    const double K = std::exp(-pCap * M_W / rhoLR / R / T);
    const double dK_dT = pCap * M_W / rhoLR / R / T / T * K;

    // vapour pressure inside porespace (== water partial pressure in gas phase)
    pWGR = p_vap_flat * K;

    auto const dp_vap_dT = dp_vap_flat_dT * K + p_vap_flat * dK_dT;

    // gas phase molar fractions
    xnWG = std::clamp(pWGR / pGR, 0., 1.);
    xnCG = 1. - xnWG;

    // molar mass of the gas phase as a mixture of 'air' and vapour
    auto const MG = xnCG * M_C + xnWG * M_W;
    variables[static_cast<int>(MaterialPropertyLib::Variable::molar_mass)] = MG;

    // gas phase mixture density
    rhoGR = gas_phase.property(MaterialPropertyLib::PropertyType::density)
                .template value<double>(variables, pos, t, dt);

    auto const drhoGR_dpGR =
        gas_phase.property(MaterialPropertyLib::PropertyType::density)
            .template dValue<double>(
                variables, MaterialPropertyLib::Variable::phase_pressure, pos,
                t, dt);

    auto const drhoGR_dT =
        gas_phase.property(MaterialPropertyLib::PropertyType::density)
            .template dValue<double>(variables,
                                     MaterialPropertyLib::Variable::temperature,
                                     pos, t, dt);

    // gas phase mass fractions
    xmCG = xnCG * M_C / MG;
    xmWG = 1. - xmCG;

    auto beta_pGR = 1. / rhoGR * drhoGR_dpGR;
    dxmWG_dpGR = xmWG * beta_pGR;
    dxmCG_dpGR = -dxmWG_dpGR;

    auto beta_TGR = -1. / rhoGR * drhoGR_dT;

    // component partial densities in the gas phase
    rhoCGR = xmCG * rhoGR;
    rhoWGR = xmWG * rhoGR;

    auto drhoWGR_dT = M_W / R / T / T * (T * dp_vap_dT - pWGR);

    dxmWG_dT = 1. / rhoGR * drhoWGR_dT + xmWG * beta_TGR;
    dxmCG_dT = -dxmWG_dT;

    // specific heat capacities of dry air and vapour
    auto const cpCG =
        dry_air_component
            .property(MaterialPropertyLib::PropertyType::specific_heat_capacity)
            .template value<double>(variables, pos, t, dt);
    auto const cpWG =
        vapour_component
            .property(MaterialPropertyLib::PropertyType::specific_heat_capacity)
            .template value<double>(variables, pos, t, dt);

    // specific enthalpy of dry air and vapour components
    hCG = cpCG * T;
    hWG = cpWG * T + dh_evap;

    // specific enthalpy of gas phase and derivatives
    hG = xmCG * hCG + xmWG * (hWG);

    // specific heat capacities of liquid phase
    auto const cpL =
        liquid_phase
            .property(MaterialPropertyLib::PropertyType::specific_heat_capacity)
            .template value<double>(variables, pos, t, dt);

    // specific enthalpy of liquid phase and derivatives
    hL = cpL * T;

    // specific inner energies of gas and liquid phases
    uG = hG - pGR / rhoGR;
    uL = hL;

    diffusion_coefficient_vapour =
        vapour_component.property(MaterialPropertyLib::PropertyType::diffusion)
            .template value<double>(variables, pos, t, dt);

    // gas phase viscosity
    muGR = gas_phase.property(MaterialPropertyLib::PropertyType::viscosity)
               .template value<double>(variables, pos, t, dt);

    // gas phase thermal conductivity
    lambdaGR =
        gas_phase
            .property(MaterialPropertyLib::PropertyType::thermal_conductivity)
            .template value<double>(variables, pos, t, dt);

    // liquid phase viscosity
    muLR = liquid_phase.property(MaterialPropertyLib::PropertyType::viscosity)
               .template value<double>(variables, pos, t, dt);

    // liquid phase thermal conductivity
    lambdaLR =
        liquid_phase
            .property(MaterialPropertyLib::PropertyType::thermal_conductivity)
            .template value<double>(variables, pos, t, dt);
}

}  // namespace TH2M
}  // namespace ProcessLib