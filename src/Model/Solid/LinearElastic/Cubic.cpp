#include "Util/Util.H"
#include "Cubic.H"

namespace Model
{
namespace Solid
{
namespace LinearElastic
{

Cubic::Cubic(Set::Scalar C11, Set::Scalar C12, Set::Scalar C44, Eigen::Matrix3d R)
{
	define(C11, C12, C44, R);
}
Cubic::Cubic(Set::Scalar C11, Set::Scalar C12, Set::Scalar C44, Set::Scalar phi1, Set::Scalar Phi, Set::Scalar phi2)
{
	define(C11, C12, C44, phi1, Phi, phi2);
}

void
Cubic::define(Set::Scalar C11, Set::Scalar C12, Set::Scalar C44, Set::Scalar phi1, Set::Scalar Phi, Set::Scalar phi2)
{
	Eigen::Matrix3d m;
	m =     Eigen::AngleAxisd(phi2, Eigen::Vector3d::UnitX()) *
		Eigen::AngleAxisd(Phi,  Eigen::Vector3d::UnitZ()) *
	 	Eigen::AngleAxisd(phi1, Eigen::Vector3d::UnitX());
	define(C11,C12,C44,m);
}
void
Cubic::define(Set::Scalar C11, Set::Scalar C12, Set::Scalar C44, Eigen::Matrix3d R)
{
  
	amrex::Real Ctmp[3][3][3][3];
	amrex::Real Crot[3][3][3][3];


	for(int i = 0; i < 3; i++) 
		for(int j = 0; j < 3; j++) 
			for(int k = 0; k < 3; k++) 
				for(int l = 0; l < 3; l++)
				{
					if(i == j && j == k && k == l)  Ctmp[i][j][k][l] = C11;
					else if (i==k && j==l) Ctmp[i][j][k][l] = C44;
					else if (i==j && k==l) Ctmp[i][j][k][l] = C12;
					else Ctmp[i][j][k][l] = 0.0;
				}

	for(int p = 0; p < 3; p++) 
		for(int q = 0; q < 3; q++) 
			for(int s = 0; s < 3; s++) 
				for(int t = 0; t < 3; t++)
				{
					Crot[p][q][s][t] = 0.0;
					for(int i = 0; i < 3; i++) 
						for(int j = 0; j < 3; j++) 
							for(int k = 0; k < 3; k++) 
								for(int l = 0; l < 3; l++) 
									Crot[p][q][s][t] += R(p,i)*R(s,k)*Ctmp[i][j][k][l]*R(q,j)*R(t,l);
				}

	C[ 0] = Crot[0][0][0][0]; C[ 1] = Crot[0][0][1][1]; C[ 2] = Crot[0][0][2][2]; C[ 3] = Crot[0][0][1][2]; C[ 4] = Crot[0][0][2][0]; C[ 5] = Crot[0][0][0][1];
	/**/                      C[ 6] = Crot[1][1][1][1]; C[ 7] = Crot[1][1][2][2]; C[ 8] = Crot[1][1][1][2]; C[ 9] = Crot[1][1][2][0]; C[10] = Crot[1][1][0][1];
	/**/                                                C[11] = Crot[2][2][2][2]; C[12] = Crot[2][2][1][2]; C[13] = Crot[2][2][2][0]; C[14] = Crot[2][2][0][1];
	/**/                                                                          C[15] = Crot[1][2][1][2]; C[16] = Crot[1][2][2][0]; C[17] = Crot[1][2][0][1];
	/**/                                                                                                    C[18] = Crot[2][0][2][0]; C[19] = Crot[2][0][0][1];
	/**/                                                                                                                              C[20] = Crot[0][1][0][1];

}

void
Cubic::Randomize()
{
	Set::Scalar C11 = Util::Random();
	Set::Scalar C12 = Util::Random();
	Set::Scalar C44 = Util::Random();

	Set::Scalar phi1 = 2.0*Set::Constant::Pi * Util::Random();
	Set::Scalar Phi  = 2.0*Set::Constant::Pi * Util::Random();
	Set::Scalar phi2 = 2.0*Set::Constant::Pi * Util::Random();

	define(C11,C12,C44,phi1,Phi,phi2);
}


#if AMREX_SPACEDIM==2
Set::Matrix
Cubic::operator () (Set::Matrix &gradu) const
{
	Set::Matrix eps = 0.5*(gradu + gradu.transpose());
	Set::Matrix sig = Set::Matrix::Zero();
		
	sig(0,0) = C[ 0]*eps(0,0) + C[ 1]*eps(1,1) /*+ C[ 2]*eps(2,2)*/ + 2.0*( /*C[ 3]*eps(1,2) + C[ 4]*eps(2,0) +*/ C[ 5]*eps(0,1));
	sig(1,1) = C[ 1]*eps(0,0) + C[ 6]*eps(1,1) /*+ C[ 7]*eps(2,2)*/ + 2.0*( /*C[ 8]*eps(1,2) + C[ 9]*eps(2,0) +*/ C[10]*eps(0,1));
	sig(0,1) = C[ 5]*eps(0,0) + C[10]*eps(1,1) /*+ C[14]*eps(2,2)*/ + 2.0*( /*C[17]*eps(1,2) + C[19]*eps(2,0) +*/ C[20]*eps(0,1));
	sig(1,0) = sig(0,1);

	return sig;
}
Set::Vector
Cubic::operator () (std::array<Set::Matrix,2> &gradgradu)
{
	std::array<Set::Matrix,2> gradeps;
	gradeps[0](0,0) = gradgradu[0](0,0);
	gradeps[0](0,1) = gradgradu[0](0,1);
	gradeps[0](1,0) = 0.5*(gradgradu[0](1,0) + gradgradu[1](0,0));
	gradeps[0](1,1) = 0.5*(gradgradu[0](1,1) + gradgradu[1](0,1));
	gradeps[1](0,0) = 0.5*(gradgradu[1](0,0) + gradgradu[0](1,0));
	gradeps[1](0,1) = 0.5*(gradgradu[1](0,1) + gradgradu[0](1,1));
	gradeps[1](1,0) = gradgradu[1](1,0);
	gradeps[1](1,1) = gradgradu[1](1,1);
	Set::Vector f = Set::Vector::Zero();

	f(0) =  C[ 0]*gradeps[0](0,0) + C[ 1]*gradeps[1](1,0) + 2.0 * ( C[ 5]*gradeps[0](1,0)) + 
		C[ 5]*gradeps[0](0,1) + C[10]*gradeps[1](1,1) + 2.0 * ( C[20]*gradeps[0](1,1));

	f(1) =  C[ 5]*gradeps[0](0,0) + C[10]*gradeps[1](1,0) + 2.0 * ( C[20]*gradeps[0](1,0)) +
		C[ 1]*gradeps[0](0,1) + C[ 6]*gradeps[1](1,1) + 2.0 * ( C[10]*gradeps[0](1,1));

	return f;
}
#elif AMREX_SPACEDIM==3
Set::Matrix
Cubic::operator () (Set::Matrix &gradu) const
{
	Set::Matrix eps = 0.5*(gradu + gradu.transpose());
	Set::Matrix sig = Set::Matrix::Zero();
		
	sig(0,0) = C[ 0]*eps(0,0) + C[ 1]*eps(1,1) + C[ 2]*eps(2,2) + 2.0 * ( C[ 3]*eps(1,2) + C[ 4]*eps(2,0) + C[ 5]*eps(0,1));
	sig(1,1) = C[ 1]*eps(0,0) + C[ 6]*eps(1,1) + C[ 7]*eps(2,2) + 2.0 * ( C[ 8]*eps(1,2) + C[ 9]*eps(2,0) + C[10]*eps(0,1));
	sig(2,2) = C[ 2]*eps(0,0) + C[ 7]*eps(1,1) + C[11]*eps(2,2) + 2.0 * ( C[12]*eps(1,2) + C[13]*eps(2,0) + C[14]*eps(0,1));
	sig(1,2) = C[ 3]*eps(0,0) + C[ 8]*eps(1,1) + C[12]*eps(2,2) + 2.0 * ( C[15]*eps(1,2) + C[16]*eps(2,0) + C[17]*eps(0,1));
	sig(2,0) = C[ 4]*eps(0,0) + C[ 9]*eps(1,1) + C[13]*eps(2,2) + 2.0 * ( C[16]*eps(1,2) + C[18]*eps(2,0) + C[19]*eps(0,1));
	sig(0,1) = C[ 5]*eps(0,0) + C[10]*eps(1,1) + C[14]*eps(2,2) + 2.0 * ( C[17]*eps(1,2) + C[19]*eps(2,0) + C[20]*eps(0,1));

	sig(2,1) = sig(1,2);
	sig(0,2) = sig(2,0);
	sig(1,0) = sig(0,1);

	return sig;
}
Set::Vector
Cubic::operator () (std::array<Set::Matrix,3> &gradgradu)
{
	std::array<Set::Matrix,3> gradeps;
	gradeps[0](0,0) = gradgradu[0](0,0);
	gradeps[0](0,1) = gradgradu[0](0,1);
	gradeps[0](0,2) = gradgradu[0](0,2);
	gradeps[0](1,0) = 0.5*(gradgradu[0](1,0) + gradgradu[1](0,0));
	gradeps[0](1,1) = 0.5*(gradgradu[0](1,1) + gradgradu[1](0,1));
	gradeps[0](1,2) = 0.5*(gradgradu[0](1,2) + gradgradu[1](0,2));
	gradeps[0](2,0) = 0.5*(gradgradu[0](2,0) + gradgradu[2](0,0));
	gradeps[0](2,1) = 0.5*(gradgradu[0](2,1) + gradgradu[2](0,1));
	gradeps[0](2,2) = 0.5*(gradgradu[0](2,2) + gradgradu[2](0,2));
	gradeps[1](0,0) = 0.5*(gradgradu[1](0,0) + gradgradu[0](1,0));
	gradeps[1](0,1) = 0.5*(gradgradu[1](0,1) + gradgradu[0](1,1));
	gradeps[1](0,2) = 0.5*(gradgradu[1](0,2) + gradgradu[0](1,2));
	gradeps[1](1,0) = gradgradu[1](1,0);
	gradeps[1](1,1) = gradgradu[1](1,1);
	gradeps[1](1,2) = gradgradu[1](1,2);
	gradeps[1](2,0) = 0.5*(gradgradu[1](2,0) + gradgradu[2](1,0));
	gradeps[1](2,1) = 0.5*(gradgradu[1](2,1) + gradgradu[2](1,1));
	gradeps[1](2,2) = 0.5*(gradgradu[1](2,2) + gradgradu[2](1,2));
	gradeps[2](0,0) = 0.5*(gradgradu[2](0,0) + gradgradu[0](2,0));
	gradeps[2](0,1) = 0.5*(gradgradu[2](0,1) + gradgradu[0](2,1));
	gradeps[2](0,2) = 0.5*(gradgradu[2](0,2) + gradgradu[0](2,2));
	gradeps[2](1,0) = 0.5*(gradgradu[2](1,0) + gradgradu[1](2,0));
	gradeps[2](1,1) = 0.5*(gradgradu[2](1,1) + gradgradu[1](2,1));
	gradeps[2](1,2) = 0.5*(gradgradu[2](1,2) + gradgradu[1](2,2));
	gradeps[2](2,0) = gradgradu[2](2,0);
	gradeps[2](2,1) = gradgradu[2](2,1);
	gradeps[2](2,2) = gradgradu[2](2,2);

	Set::Vector f = Set::Vector::Zero();

	f(0) =  C[ 0]*gradeps[0](0,0) + C[ 1]*gradeps[1](1,0) + C[ 2]*gradeps[2](2,0) + 2.0 * ( C[ 3]*gradeps[1](2,0) + C[ 4]*gradeps[2](0,0) + C[ 5]*gradeps[0](1,0)) + 
		C[ 5]*gradeps[0](0,1) + C[10]*gradeps[1](1,1) + C[14]*gradeps[2](2,1) + 2.0 * ( C[17]*gradeps[1](2,1) + C[19]*gradeps[2](0,1) + C[20]*gradeps[0](1,1)) + 
		C[ 4]*gradeps[0](0,2) + C[ 9]*gradeps[1](1,2) + C[13]*gradeps[2](2,2) + 2.0 * ( C[16]*gradeps[1](2,2) + C[18]*gradeps[2](0,2) + C[19]*gradeps[0](1,2));

	f(1) =  C[ 5]*gradeps[0](0,0) + C[10]*gradeps[1](1,0) + C[14]*gradeps[2](2,0) + 2.0 * ( C[17]*gradeps[1](2,0) + C[19]*gradeps[2](0,0) + C[20]*gradeps[0](1,0)) +
		C[ 1]*gradeps[0](0,1) + C[ 6]*gradeps[1](1,1) + C[ 7]*gradeps[2](2,1) + 2.0 * ( C[ 8]*gradeps[1](2,1) + C[ 9]*gradeps[2](0,1) + C[10]*gradeps[0](1,1)) +
		C[ 3]*gradeps[0](0,2) + C[ 8]*gradeps[1](1,2) + C[12]*gradeps[2](2,2) + 2.0 * ( C[15]*gradeps[1](2,2) + C[16]*gradeps[2](0,2) + C[17]*gradeps[0](1,2));

	f(2) =  C[ 4]*gradeps[0](0,0) + C[ 9]*gradeps[1](1,0) + C[13]*gradeps[2](2,0) + 2.0 * ( C[16]*gradeps[1](2,0) + C[18]*gradeps[2](0,0) + C[19]*gradeps[0](1,0)) +
		C[ 3]*gradeps[0](0,1) + C[ 8]*gradeps[1](1,1) + C[12]*gradeps[2](2,1) + 2.0 * ( C[15]*gradeps[1](2,1) + C[16]*gradeps[2](0,1) + C[17]*gradeps[0](1,1)) + 
		C[ 2]*gradeps[0](0,2) + C[ 7]*gradeps[1](1,2) + C[11]*gradeps[2](2,2) + 2.0 * ( C[12]*gradeps[1](2,2) + C[13]*gradeps[2](0,2) + C[14]*gradeps[0](1,2));
	return f;
}
#endif

}
}
}
