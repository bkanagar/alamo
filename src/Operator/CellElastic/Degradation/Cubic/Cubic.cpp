#include <AMReX_MultiFabUtil.H>
#include <AMReX_REAL.H>
#include <AMReX_MLCGSolver.H>
#include <AMReX_ArrayLim.H>

#include "Cubic.H"



Operator::CellElastic::Degradation::Cubic::Cubic (amrex::Real _C11, amrex::Real _C12, amrex::Real _C44,
					      amrex::Real phi1=0.0, amrex::Real Phi=0.0, amrex::Real phi2=0.0):
  C11(_C11), C12(_C12), C44(_C44)
{
  R =
    Eigen::AngleAxisd(phi2,Eigen::Vector3d::UnitZ()) *
    Eigen::AngleAxisd(Phi,Eigen::Vector3d::UnitX()) *
    Eigen::AngleAxisd(phi1,Eigen::Vector3d::UnitZ());
}

amrex::Real
Operator::CellElastic::Degradation::Cubic::C(const int p, const int q, const int s, const int t) const
{

  amrex::Real Crot = 0.0;

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      for (int k = 0; k < 3; k++)
	for (int l = 0; l < 3; l++)
	  {
	    amrex::Real C = 0.0;
	    if (i==j && j==k && k==l) C = C11;
	    else if (i==j && k==l)    C = C12;
	    else if (i==k && j==l)    C = C44;

	    Crot += R.transpose()(i,p) * R.transpose()(k,s) * C * R(q,j) * R(t,l);

	  }

  return Crot;

}
