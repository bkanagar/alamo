#ifndef OPERATOR_CELLELASTIC_DEGRADATION_H_
#define OPERATOR_CELLELASTIC_DEGRADATION_H_

#include <AMReX_MLCellLinOp.H>
#include <AMReX_Array.H>
#include <limits>

#include "BC/BC.H"
#include "Operator/CellElastic/CellElastic.H"

using namespace amrex;

namespace Operator
{
namespace CellElastic
{
namespace Degradation
{

class PristineMaterialModel
{
	friend class Degradation;
	virtual amrex::Real C(const int i, const int j, const int k, const int l) const = 0;
};

class Degradation : public CellElastic
{
public:

	Degradation () {};
	Degradation(const amrex::Real WInput);
	virtual ~Degradation () {};
	void SetEta(amrex::Vector<std::unique_ptr<amrex::MultiFab> > &eta, std::vector<PristineMaterialModel *> &models);
	void Energies (FArrayBox& energyfab,
		const FArrayBox& ufab,
		int amrlev, int mglev, const MFIter& mfi);

protected:

	int num_eta = 1;
	virtual amrex::Real C(const int i, const int j, const int k, const int l, const amrex::IntVect loc,
		const int amrlev, const int mglev, const MFIter &mfi) const;
	virtual amrex::Real M(const int i, const int j, const int k, const int l, const amrex::IntVect loc,
		const int amrlev, const int mglev, const MFIter &mfi) const;
	virtual amrex::Real N(const int i, const int j, const amrex::IntVect loc, const int amrlev,
		const int mglev, const MFIter &mfi) const;
	virtual std::vector<amrex::Real> dMdEta(const int i, const int j, const int k, const int l,
		const amrex::IntVect loc, const int amrlev, const int mglev, const MFIter &mfi) const;
	virtual std::vector<amrex::Real> dNdEta(const int i, const int j, const amrex::IntVect loc,
		const int amrlev, const int mglev, const MFIter &mfi) const;

	std::vector<std::array<std::array<std::array<std::array<amrex::Real,AMREX_SPACEDIM>,AMREX_SPACEDIM>,AMREX_SPACEDIM>,AMREX_SPACEDIM> > Cs;

private:
	amrex::Real W;
};
}
}
}
#endif
