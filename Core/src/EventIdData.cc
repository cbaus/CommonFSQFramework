#include "CommonFSQFramework/Core/interface/EventIdData.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "TLorentzVector.h"

EventIdData::EventIdData(const edm::ParameterSet& iConfig, TTree * tree):
EventViewBase(iConfig, tree)
{

    registerInt("run", tree);
    registerInt("lumi", tree);
    registerInt("event", tree);
    registerFloat("genWeight", tree);

    registerFloat("alphaQCD", tree);
    registerFloat("qScale", tree);
    
    registerInt("processID", tree);
    registerFloat("Xix", tree);
    registerFloat("Xiy", tree);
    registerFloat("XiSD", tree);
    registerFloat("XiDD", tree);
    

    registerFloat("puTrueNumInteractions", tree);
    registerFloat("PUNumInteractions", tree);

    localcount = 0;

}


void EventIdData::fillSpecific(const edm::Event& iEvent, const edm::EventSetup& iSetup){

    setI("run", iEvent.eventAuxiliary().run());
    setI("lumi", iEvent.eventAuxiliary().luminosityBlock());
    setI("event", iEvent.eventAuxiliary().event());

    if (iEvent.isRealData()) return;

    // MC only stuff below
    edm::Handle<GenEventInfoProduct> hGW; 
    iEvent.getByLabel(edm::InputTag("generator"), hGW);
    setF("genWeight", hGW->weight());
    setF("alphaQCD", hGW->alphaQCD());
    setF("qScale", hGW->qScale());
    setI("processID",hGW->signalProcessID());
    
    // calculate the Xi's of the event
    double xix = 100;
    double xiy = 100;
    double xisd = 100;
    double xidd = 10e10;
    
    // get the HepMC information
    edm::Handle<edm::HepMCProduct> hepmc;
    iEvent.getByLabel("generator", hepmc);
    
    std::vector<TLorentzVector> myTempParticles;
    std::vector<TLorentzVector> myRapiditySortedParticles;
    
    // copy only final state particles from the genParticles collection
    for(HepMC::GenEvent::particle_const_iterator p = hepmc->GetEvent()->particles_begin(); p != hepmc->GetEvent()->particles_end(); ++p) {
    	if ((*p)->status() != 1) continue;
	TLorentzVector vp;
	vp.SetPxPyPzE((*p)->momentum().px(),(*p)->momentum().py(),(*p)->momentum().pz(),(*p)->momentum().e());
    	myTempParticles.push_back(vp);
    }
    		
    // sort genParticles in y, from y_min to y_max
    while (myTempParticles.size() != 0) {
    	double min_y = 10000;
	int min_y_pos = -1;
	for (unsigned int ipart = 0;ipart<myTempParticles.size();ipart++) {
		if (myTempParticles[ipart].Rapidity() < min_y) {
			min_y = myTempParticles[ipart].Rapidity();
			min_y_pos = ipart;
		}
	}
	myRapiditySortedParticles.push_back(myTempParticles[min_y_pos]);
	myTempParticles.erase(myTempParticles.begin()+min_y_pos);
    }
			
    // find deltaymax
    double deltaymax = 0;
    int deltaymax_pos = -1;
    for (unsigned int ipart=0;ipart<myRapiditySortedParticles.size()-1;ipart++) {
    	double deltay = myRapiditySortedParticles[ipart+1].Rapidity() - myRapiditySortedParticles[ipart].Rapidity();
	if (deltay > deltaymax) {
		deltaymax = deltay;
		deltaymax_pos = ipart;
	}
    }
			
    // calculate Mx2 and My2
    long double XEtot = 0;
    long double XPxtot = 0;
    long double XPytot = 0;
    long double XPztot = 0;
    long double YEtot = 0;
    long double YPxtot = 0;
    long double YPytot = 0;
    long double YPztot = 0;
			
    for (int ipart=0;ipart<=deltaymax_pos;ipart++) {
    	XEtot += myRapiditySortedParticles[ipart].E();
	XPxtot += myRapiditySortedParticles[ipart].Px();
	XPytot += myRapiditySortedParticles[ipart].Py();
	XPztot += myRapiditySortedParticles[ipart].Pz();
    }
    long double Mx2 = -1.;
    Mx2 = XEtot*XEtot - XPxtot*XPxtot - XPytot*XPytot - XPztot*XPztot;
			
    for (unsigned int ipart=deltaymax_pos+1;ipart<myRapiditySortedParticles.size();ipart++) {
    	YEtot += myRapiditySortedParticles[ipart].E();
	YPxtot += myRapiditySortedParticles[ipart].Px();
	YPytot += myRapiditySortedParticles[ipart].Py();
	YPztot += myRapiditySortedParticles[ipart].Pz();
    }
    long double My2 = -1.;
    My2 = YEtot*YEtot - YPxtot*YPxtot - YPytot*YPytot - YPztot*YPztot;
			
    // get the centre-of-mass energy from the event
    double cmenergy = 0;
    cmenergy = std::abs(hepmc->GetEvent()->beam_particles().first->momentum().pz()) + std::abs(hepmc->GetEvent()->beam_particles().second->momentum().pz());
    			
    // calculate xix and xiy
    xix = Mx2/(cmenergy*cmenergy);
    xiy = My2/(cmenergy*cmenergy);
			
    // xisd and xidd
    xisd = std::max(xix,xiy);
    xidd = xix*xiy*cmenergy*cmenergy/(0.938*0.938);
    
    // fill tree
    setF("Xix",xix);
    setF("Xiy",xiy);
    setF("XiSD",xisd);
    setF("XiDD",xidd);
    
    
    // pile-up stuff

    try {
    	edm::Handle< std::vector<PileupSummaryInfo> > hPU;
    	iEvent.getByLabel(edm::InputTag("addPileupInfo"), hPU);
    	for (unsigned int i = 0; i< hPU->size();++i){
        	/*
        	std::cout << hPU->at(i).getBunchCrossing() 
                  << " " << hPU->at(i).getTrueNumInteractions() 
                  << " " << hPU->at(i).getPU_NumInteractions() << std::endl;
        	*/
        	if (hPU->at(i).getBunchCrossing() == 0) {
            		setF("puTrueNumInteractions",  hPU->at(i).getTrueNumInteractions());
            		setF("PUNumInteractions",  hPU->at(i).getPU_NumInteractions());
            		break;
        	}
    	}
    } catch (...) {
    	if (localcount == 0) std::cout << " An exception was thrown when accessing the PileupSummaryInfo object. This means we are probably running on GEN-SIM samples that do not have this information." << std::endl;
    }

    localcount++;

}
