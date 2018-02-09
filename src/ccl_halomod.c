
#include "ccl_utils.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "gsl/gsl_integration.h"
#include "gsl/gsl_interp.h"
#include "gsl/gsl_spline.h"
#include "gsl/gsl_sf_expint.h"
//#include "gsl/gsl_interp2d.h"
//#include "gsl/gsl_spline2d.h"
#include "ccl_placeholder.h"
#include "ccl_background.h"
#include "ccl_power.h"
#include "ccl_massfunc.h"
#include "ccl_error.h"
#include "class.h"
#include "ccl_params.h"
#include "ccl_emu17.h"
#include "ccl_emu17_params.h"

static double r_delta(ccl_cosmology *cosmo, double halomass, double a, int * status){
  double rho_m, delta;
  rho_m = RHO_CRITICAL*cosmo->params.Omega_m*cosmo->params.h*cosmo->params.h;
  delta = 200.0;
  return pow(halomass*3.0/(4.0*M_PI*rho_m*delta),1.0/3.0);
}

// TODO: rename parameters for consistency.
// TODO: check if r_Delta is something equivalent in CCL.
double u_nfw_c(ccl_cosmology *cosmo, double c,double halomass, double k, double a, int * status){
  // analytic FT of NFW profile, from Cooray & Sheth 2001
  double x, xu;
  double f1, f2, f3, fc;
  x = k * r_delta(cosmo, halomass, a, status)/c; // x = k*rv/c = k*rs = ks
  xu = (1.+c)*x; // xu = ks*(1+c)
  f1 = sin(x)*(gsl_sf_Si(xu)-gsl_sf_Si(x));
  f2 = cos(x)*(gsl_sf_Ci(xu)-gsl_sf_Ci(x));
  f3 = sinl(c*x)/xu;
  fc = log(1.+c)-c/(1.+c);
  return (f1+f2-f3)/fc;
}

// TODO: we should probably set this up as a macro..
double delta_c() {
  //Linear collapse threshold
  return 1.686;
}

// TODO: move this into ccl_massfunc as standard function conversion.
double nu(ccl_cosmology *cosmo, double halomass, double a, int * status) {
  //nu = delta_c/sigma(M)
  return delta_c()/ccl_sigmaM(cosmo,halomass,a,status);
}

// TODO: make consistency check so that ccl_halo_concentration only runs if called with appropriate definition
// e.g. if Delta != 200 rho_{mean}, should not function.
double ccl_halo_concentration(ccl_cosmology *cosmo, double halomass, double a, int * status)
{
  // Bhattacharya et al. 2011, Delta = 200 rho_{mean} (Table 2)
  return 9.*pow(nu(cosmo,halomass,a,status),-.29)*pow(ccl_growth_factor(cosmo, a, status)/ccl_growth_factor(cosmo, 1.0, status),1.15);
  //return 10.14*pow(m/2.e+12,-0.081)*pow(a,1.01); //Duffy et al. 2008 (Delta = 200 mean)
}

// TODO: move this into ccl_massfunc - be careful about the units!
double massfunc(double nu) {
  //Sheth Tormen mass function!
  //Note that nu=dc/sigma(M) and this Sheth & Tormen (1999) use nu=(dc/sigma)^2
  //This accounts for some small differences
  double p=0.3;
  double q=0.707;
  double A=0.21616;
  return A*(1.+pow(q*nu*nu,-p))*exp(-q*nu*nu/2.);
}

double inner_I0j (ccl_cosmology *cosmo, double halomass, double k, double a, void *para, int * status){
  double *array = (double *) para;
  long double u = 1.0; //the number one?
  double arr= array[6]; //Array of ...
  double c = ccl_halo_concentration(cosmo, halomass,a, status); //The halo concentration for this mass and scale factor
  int l;
  int j = (int)(array[5]);
  for (l = 0; l< j; l++){
    u = u*u_nfw_c(cosmo, c, halomass, k, a, status);
  }
  // TODO: mass function should be the CCL call - check units due to changes (Msun vs Msun/h, etc)
  return massfunc(nu(cosmo,halomass,a,status))*halomass*pow(halomass/(RHO_CRITICAL*cosmo->params.Omega_m),(double)j)*u;
}

// TODO: we need to change this to the CCL style integrator.
// TODO: pass the cosmology construct.
double I0j (int j, double k1, double k2, double k3, double k4,double a){
  double array[7] = {k1,k2,k3,k4,0.,(double)j,a};
//  return int_gsl_integrate_medium_precision(inner_I0j,(void*)array,log(limits.M_min),log(limits.M_max),NULL, 2000);
  return 1.0; //temp while refactoring.
}

// TODO: pass the cosmology construct around.
double p_1h(double k, double a)
{
  return I0j(2,k,k,0.,0.,a);
}



// TODO: referred to as odelta in ccl_massfunc, as a free parameter.
double Delta_v() {
  //Halo mean density
  return 200.;
}
