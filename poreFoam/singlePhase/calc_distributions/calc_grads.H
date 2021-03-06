/*-------------------------------------------------------------------------*\
This code is part of poreFOAM, a suite of codes written using OpenFOAM
for direct simulation of flow at the pore scale. 	
You can redistribute this code and/or modify this code under the 
terms of the GNU General Public License (GPL) as published by the  
Free Software Foundation, either version 3 of the License, or (at 
your option) any later version. see <http://www.gnu.org/licenses/>.



The code has been developed by Ali Qaseminejad Raeini as a part his PhD 
at Imperial College London, under the supervision of Branko Bijeljic 
and Martin Blunt. 
Please see our website for relavant literature:
http://www3.imperial.ac.uk/earthscienceandengineering/research/perm/porescalemodelling

For further information please contact us by email:
Ali Q Raeini:    a.qaseminejad-raeini09@imperial.ac.uk

 Description:
	post-processing code Calculates energy losses for control 
	volumes given in postprossingDict file in $case/system directory. 
	
	Note: Parallel case post-processing is added to the code, no need to 
	reconstract a parallel case anymore.
\*-------------------------------------------------------------------------*/






//~ int PostProcess(const fvMesh& mesh)
{
	Info<<endl;











	//int counter = 0;
	vector uTotal(0.0,0.0,0.0);

	//~ forAll(timeDirs, timeI)
	//~ if (timeDirs[timeI]==0.0671834)
	{
	//~ runTime.setTime(timeDirs[timeI], timeI);

		snapShot_t data(CVBounds1.size());

		data.t=runTime.value();

		volVectorField gradP=	fvc::grad(p); 

		volScalarField Uz=U.component(0);
		volVectorField gUz=fvc::grad(Uz);
		volScalarField gradPz=gradP.component(0);


		volScalarField ViscERate=(fvc::laplacian(muEff,U) & U);
		volScalarField gradPERate=-gradP & U;
		volScalarField phiERate=-fvc::div(rho*phi, U) & U ;






	
		data.maxMagU= max(mag(U)).value();

		data.QIn=gSum(U.boundaryField()[iBack]&U.boundaryField()[iBack].patch().Sf());
		data.QOut=gSum(U.boundaryField()[iFront]&U.boundaryField()[iFront].patch().Sf());
		
		
		data.Dp=(gAverage(p.boundaryField()[iBack])-gAverage(p.boundaryField()[iFront]));


		data.ADarcy=(max(mesh.C().component(2)).value()-min(mesh.C().component(2)).value())*(max(mesh.C().component(1)).value()-min(mesh.C().component(1)).value());




		const vectorField & C =	mesh.C().internalField();

		for (int iSam=0;iSam<regionVoxelValues.size();iSam++)   
		{
			snapShot_zt & slice = data.slices[iSam];
			double xSam1=CVBounds1[iSam];
			double xSam2=CVBounds2[iSam];

			Info<<iSam<< "   Del x :"<< xSam1 <<" to "<< xSam2<< "   ";		

			//~ clip1=0.0;
			clip=0.0;
			forAll(Cx,c) 
			{ 
					if (PPRegions[c]==regionVoxelValues[iSam]&& 
						(C[c][0]>=CVBounds1[iSam] && C[c][0]<=CVBounds2[iSam] && 
						 C[c][1]>=CVyBounds1[iSam] && C[c][1]<=CVyBounds2[iSam] && 
						 C[c][2]>=CVzBounds1[iSam] && C[c][2]<=CVzBounds2[iSam] ))
					   clip[c]=1.0;
			}


			slice.x1()=xSam1;		
			slice.x2()=xSam2;
			scalar delX=xSam2-xSam1;
			Info<<"   ->         "<<slice.x1()<<"  "<<slice.x2()<<endl;		




			volScalarField clipVC=clip*VC;
			const fvBoundaryMesh& boundary = mesh.boundary();

			forAll(boundary, patchi)
			{
				clip.boundaryFieldRef()[patchi]=0.0;
				clip.boundaryFieldRef()[patchi]==0.0;
			}



			scalar vol=sum(clipVC).value();
			
			if(mag(vol)<1.0e-31) 
			{
				Info<<"Error in post-processing: CV volume is zero: label "<<regionVoxelValues[iSam]<<endl;
				vol=1.0e-31;
			}
			slice.vol()=vol;

			slice.q()=sum(Uz *clipVC).value()/vol; if(mag(slice.q())<1.0e-32) slice.q()=1.0e-32;
			slice.dpddz()=-sum(gradPz *clipVC).value()/vol;

			#include  "calcdEdts.H"


			slice.viscz()=viscE[2]/vol;
			slice.phiu()=rhoPhiUE[2]/vol;
			slice.delPdelX()=gPdE[2]/vol;




			slice.viscE()=sum( ViscERate  *clipVC).value()/vol;
			slice.dpdE()=sum( gradPERate *clipVC).value()/vol;
			slice.phiE()=sum( phiERate *clipVC).value()/vol;


			Info<< "viscE phiE dpdE :  "
				 << slice.viscE()*vol/(slice.q()*vol/delX)<< "    " 
				 << slice.phiE()*vol/(slice.q()*vol/delX)   << "      " 
				 << slice.dpdE()*vol/(slice.q()*vol/delX)   <<endl;



		}

		if (Pstream::master())
		{
			data.write(data_out_for_plot);
			Info<<"wrote grads_SP.csv"<<endl;
		}
	}

	Info<<endl;
} 




