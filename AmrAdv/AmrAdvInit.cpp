
#include <AmrAdv.H>

using namespace amrex;

void
AmrAdv::InitData ()
{
    if (restart_chkfile.empty())
    {
	const Real time = 0.0;
	InitFromScratch(time);
	AverageDown();

	if (plot_int > 0) {
	    WritePlotFile();
	}
    }
    else
    {
	InitFromCheckpoint();
    }
}

void AmrAdv::MakeNewLevelFromScratch (int lev, Real time, const BoxArray& ba,
				      const DistributionMapping& dm)
{
    const int ncomp = 1;
    const int nghost = 1;

    phi_new[lev].reset(new MultiFab(ba, dm, ncomp, nghost));
    phi_old[lev].reset(new MultiFab(ba, dm, ncomp, nghost));

    t_new[lev] = time;
    t_old[lev] = time - 1.e200;

    const Real* dx = geom[lev].CellSize();
    const Real* prob_lo = geom[lev].ProbLo();
    Real cur_time = t_new[lev];

    MultiFab& state = *phi_new[lev];

    for (MFIter mfi(state,true); mfi.isValid(); ++mfi)
    {
        const Box& box = mfi.tilebox();

	amrex::BaseFab<Real> &phi_box = state[mfi];

	//double offset_x = 0.25, offset_y=0.25;
	double offset_x = 0., offset_y=0.;
	
	for (int i = box.loVect()[0]; i<=box.hiVect()[0]; i++)
	  for (int j = box.loVect()[1]; j<=box.hiVect()[1]; j++)
	    {
	      amrex::Real x = geom[lev].ProbLo()[0] + ((amrex::Real)(i) + 0.5) * geom[lev].CellSize()[0];
	      amrex::Real y = geom[lev].ProbLo()[1] + ((amrex::Real)(j) + 0.5) * geom[lev].CellSize()[1];
	      amrex::Real r = sqrt((x-offset_x)*(x-offset_x) + (y-offset_y)*(y-offset_y));
	      if (r<0.5) phi_box(amrex::IntVect(i,j)) =  2.;
	      else phi_box(amrex::IntVect(i,j)) =  1. + exp(-((r-0.5)*(r-0.5))/0.001);
	    }
    }
}
