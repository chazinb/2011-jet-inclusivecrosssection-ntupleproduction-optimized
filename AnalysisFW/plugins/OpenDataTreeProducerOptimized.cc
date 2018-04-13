

// Forked from SMPJ Analysis Framework
// https://twiki.cern.ch/twiki/bin/viewauth/CMS/SMPJAnalysisFW
// https://github.com/cms-smpj/SMPJ/tree/v1.0


#include <iostream>
#include <sstream>
#include <istream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <functional>
#include "TTree.h"
#include <vector>
#include <cassert>
#include <TLorentzVector.h>

#include "OpenDataTreeProducerOptimized.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/JetReco/interface/JetCollection.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/JetReco/interface/JetExtendedAssociation.h"
#include "DataFormats/JetReco/interface/JetFloatAssociation.h"
#include "DataFormats/JetReco/interface/JetID.h"
#include "DataFormats/METReco/interface/PFMET.h"
#include "DataFormats/METReco/interface/PFMETCollection.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/BTauReco/interface/JetTag.h"
#include "DataFormats/BTauReco/interface/JetTagInfo.h"
#include "DataFormats/BTauReco/interface/SecondaryVertexTagInfo.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/JetMatching/interface/JetFlavour.h"
#include "SimDataFormats/JetMatching/interface/JetFlavourMatching.h"
#include "SimDataFormats/JetMatching/interface/MatchedPartons.h"
#include "SimDataFormats/JetMatching/interface/JetMatchedPartons.h"
#include "SimDataFormats/JetMatching/interface/JetFlavourInfo.h"
#include "SimDataFormats/JetMatching/interface/JetFlavourInfoMatching.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/JetReco/interface/JetTracksAssociation.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "DataFormats/BTauReco/interface/TrackIPTagInfo.h"
#include "DataFormats/Math/interface/Vector3D.h"

#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"
#include "RecoJets/JetAssociationProducers/src/JetTracksAssociatorAtVertex.h"

//https://github.com/cms-sw/cmssw/blob/CMSSW_5_3_X/PhysicsTools/JetExamples/test/printJetFlavourInfo.py
//https://github.com/cms-sw/cmssw/blob/CMSSW_5_3_X/PhysicsTools/JetExamples/test/printJetFlavourInfo.cc

OpenDataTreeProducerOptimized::OpenDataTreeProducerOptimized(edm::ParameterSet const &cfg) {
  mMinPFPt           = cfg.getParameter<double>                    ("minPFPt");
  mMinJJMass         = cfg.getParameter<double>                    ("minJJMass");
  mMaxY              = cfg.getParameter<double>                    ("maxY");
  mMinNPFJets        = cfg.getParameter<int>                       ("minNPFJets");
  mPFak5JetsName     = cfg.getParameter<edm::InputTag>             ("pfak5jets");
  mPFak7JetsName     = cfg.getParameter<edm::InputTag>             ("pfak7jets");
  mOfflineVertices   = cfg.getParameter<edm::InputTag>             ("offlineVertices");
  mGoodVtxNdof       = cfg.getParameter<double>                    ("goodVtxNdof");
  mGoodVtxZ          = cfg.getParameter<double>                    ("goodVtxZ");
  mSrcPFRho          = cfg.getParameter<edm::InputTag>             ("srcPFRho");
  mPFMET             = cfg.getParameter<edm::InputTag>             ("pfmet");
  mGenJetsName       = cfg.getUntrackedParameter<edm::InputTag>    ("genjets",edm::InputTag(""));
  mPrintTriggerMenu  = cfg.getUntrackedParameter<bool>             ("printTriggerMenu",false);
  mIsMCarlo          = cfg.getUntrackedParameter<bool>             ("isMCarlo",false);
  mUseGenInfo        = cfg.getUntrackedParameter<bool>             ("useGenInfo",false);
  mMinGenPt          = cfg.getUntrackedParameter<double>           ("minGenPt",30);
  processName_       = cfg.getParameter<std::string>               ("processName");
  triggerNames_      = cfg.getParameter<std::vector<std::string> > ("triggerNames");
  triggerResultsTag_ = cfg.getParameter<edm::InputTag>             ("triggerResults");
  mJetCorr_ak5       = cfg.getParameter<std::string>               ("jetCorr_ak5");
  mJetCorr_ak7       = cfg.getParameter<std::string>               ("jetCorr_ak7");

  // test flavour
  if (mIsMCarlo){jetFlavourInfos_ = cfg.getParameter<edm::InputTag>("jetFlavourInfos");}
  // test SV
  impactParameterTagInfos_  = cfg.getParameter<edm::InputTag>("impactParameterTagInfos");
  secondaryVertexTagInfos_  = cfg.getParameter<edm::InputTag>("secondaryVertexTagInfos"); 
  // test track IP
  m_ipassoc = cfg.getParameter<edm::InputTag>("ipassociation");
}

void OpenDataTreeProducerOptimized::beginJob() {
    mTree = fs->make< TTree >("OpenDataTree", "OpenDataTree");

    // Variables of the flat tuple
    mTree->Branch("njet", &njet, "njet/i");
    mTree->Branch("jet_pt", jet_pt, "jet_pt[njet]/F");
    mTree->Branch("jet_eta", jet_eta, "jet_eta[njet]/F");
    mTree->Branch("jet_phi", jet_phi, "jet_phi[njet]/F");
    mTree->Branch("jet_E", jet_E, "jet_E[njet]/F");   
    mTree->Branch("jet_tightID", jet_tightID, "jet_tightID[njet]/O");
    mTree->Branch("jet_area", jet_area, "jet_area[njet]/F");
    mTree->Branch("jet_jes", jet_jes, "jet_jes[njet]/F");
    mTree->Branch("jet_igen", jet_igen, "jet_igen[njet]/I");
    // b discriminant
    mTree->Branch("jet_CSV", jet_CSV, "jet_CSV[njet]/F");
    mTree->Branch("jet_JBP", jet_JBP, "jet_JBP[njet]/F");
    mTree->Branch("jet_TCHP", jet_TCHP, "jet_TCHP[njet]/F");

    // AK7 variables
    mTree->Branch("njet_ak7", &njet_ak7, "njet_ak7/i");
    mTree->Branch("jet_pt_ak7", jet_pt_ak7, "jet_pt_ak7[njet_ak7]/F");
    mTree->Branch("jet_eta_ak7", jet_eta_ak7, "jet_eta_ak7[njet_ak7]/F");
    mTree->Branch("jet_phi_ak7", jet_phi_ak7, "jet_phi_ak7[njet_ak7]/F");
    mTree->Branch("jet_E_ak7", jet_E_ak7, "jet_E_ak7[njet_ak7]/F");
    mTree->Branch("jet_area_ak7", jet_area_ak7, "jet_area_ak7[njet_ak7]/F");
    mTree->Branch("jet_jes_ak7", jet_jes_ak7, "jet_jes_ak7[njet_ak7]/F");
    mTree->Branch("ak7_to_ak5", ak7_to_ak5, "ak7_to_ak5[njet_ak7]/I");

    mTree->Branch("ngen", &ngen, "ngen/i");
    mTree->Branch("gen_pt", gen_pt, "gen_pt[ngen]/F");
    mTree->Branch("gen_eta", gen_eta, "gen_eta[ngen]/F");
    mTree->Branch("gen_phi", gen_phi, "gen_phi[ngen]/F");
    mTree->Branch("gen_E", gen_E, "gen_E[ngen]/F");

    mTree->Branch("run", &run, "run/i");
    mTree->Branch("lumi", &lumi, "lumi/i");
    mTree->Branch("event", &event, "event/l");
    mTree->Branch("ntrg", &ntrg, "ntrg/i");
    mTree->Branch("triggers", triggers, "triggers[ntrg]/O");
    mTree->Branch("triggernames", &triggernames);
    mTree->Branch("prescales", prescales, "prescales[ntrg]/i");
    mTree->Branch("met", &met, "met/F");
    mTree->Branch("sumet", &sumet, "sumet/F");
    mTree->Branch("rho", &rho, "rho/F");
    mTree->Branch("pthat", &pthat, "pthat/F");
    mTree->Branch("mcweight", &mcweight, "mcweight/F");

    mTree->Branch("chf", chf, "chf[njet]/F");   
    mTree->Branch("nhf", nhf, "nhf[njet]/F");   
    mTree->Branch("phf", phf, "phf[njet]/F");   
    mTree->Branch("elf", elf, "elf[njet]/F");   
    mTree->Branch("muf", muf, "muf[njet]/F");   
    mTree->Branch("hf_hf", hf_hf, "hf_hf[njet]/F");   
    mTree->Branch("hf_phf", hf_phf, "hf_phf[njet]/F");   
    mTree->Branch("hf_hm", hf_hm, "hf_hm[njet]/i");    
    mTree->Branch("hf_phm", hf_phm, "hf_phm[njet]/i");
    mTree->Branch("chm", chm, "chm[njet]/i");   
    mTree->Branch("nhm", nhm, "nhm[njet]/i");   
    mTree->Branch("phm", phm, "phm[njet]/i");   
    mTree->Branch("elm", elm, "elm[njet]/i");   
    mTree->Branch("mum", mum, "mum[njet]/i");
    mTree->Branch("beta", beta, "beta[njet]/F");   
    mTree->Branch("bstar", bstar, "bstar[njet]/F");
    
    //loose WP for commisionning
    mTree->Branch("nhfJet", nhfJet, "nhfJet[njet]/F");
    mTree->Branch("nemfJet", nemfJet, "nemfJet[njet]/F");
    mTree->Branch("chemfJet", chemfJet, "chemfJet[njet]/F");
    mTree->Branch("chmJet", chmJet, "chmJet[njet]/i"); 
    mTree->Branch("jet_looseID", jet_looseID, "jet_looseID[njet]/O");
 
    // Test flavour  
    mTree->Branch("ptF",     ptF,    "ptF[njet]/F");    
    mTree->Branch("etaF",    etaF,   "etaF[njet]/F");    
    mTree->Branch("phiF",    phiF,   "phiF[njet]/F");    
    mTree->Branch("HadronF", HadronF,"HadronF[njet]/F");    
    mTree->Branch("PartonF", PartonF,"PartonF[njet]/F");    
    mTree->Branch("nBHadrons", nBHadrons,"nBHadrons[njet]/F");   
    // Test to get the N generated in MC, N processed in data
    mTree->Branch("nevent", &nevent,"nevent/i");   
    // Test number of Secondary Vertex
    mTree->Branch("nSVertex",     &nSVertex,    "nSVertex/i");    
    // track multiplicity
    mTree->Branch("ntrack", &ntrack, "ntrack/i");
    mTree->Branch("ntracks", ntracks, "ntracks[njet]/i");

 
}

void OpenDataTreeProducerOptimized::endJob() {
}


void OpenDataTreeProducerOptimized::beginRun(edm::Run const &iRun,
                                     edm::EventSetup const &iSetup) {

    // Mapping trigger indices 
    bool changed(true);
    if (hltConfig_.init(iRun, iSetup, processName_, changed) && changed) {

        // List of trigger names and indices 
        // are not emptied between events, must be done here
        triggerIndex_.clear();
        triggernames.clear();

        // Iterate over all active triggers of the AOD file
    

	int iTrigger=0;//mi trigger***
        auto name_list = hltConfig_.triggerNames();
        for (std::string name_to_search: triggerNames_) {
            // Find the version of jet trigger that is active in this run 
            
            int jTrigger=0; // trigger de la coleccion***
            for (std::string name_candidate: name_list) {
	   
	        //printf(" Mi trigger %i: %s y el trigger de la coleccion %i: %s\n", iTrigger++, triggerNames_, jTrigger++, name_list);
		//printf(" Mi trigger %i: %s y el trigger de la coleccion %i: %s\n", iTrigger++, name_to_search, jTrigger++, name_candidate);
                std::cout<< "Mi trigger  : " << iTrigger++ << "    name_to_search     " << name_to_search << std::endl; 
                std::cout<< "el trigger de la coleccion  : " << jTrigger++ << "    name_candidate     " << name_candidate << std::endl; 
                


                // Match the prefix to the full name (eg. HLT_Jet30 to HLT_Jet30_v10)
                if ( name_candidate.find(name_to_search + "_v") != std::string::npos ) {
                    // Save index corresponding to the trigger
                    triggerIndex_.push_back(hltConfig_.triggerIndex(name_candidate));

                    // Save the trigger name
                    triggernames.push_back("jt" + name_to_search.substr(7, string::npos));
                    break;            
                }
            }
        }
    }

    // Retrieve cross section of the simulated process
    mcweight = 0;
    if (mIsMCarlo) {

        edm::Handle<GenRunInfoProduct> genRunInfo;
        iRun.getByLabel("generator", genRunInfo );

        // Save only the cross section, since the total number of 
        // generated events is not available in this context (!!)
        mcweight = genRunInfo->crossSection();
        std::cout << "Cross section: " <<  mcweight << std::endl;
    }
    
}


void OpenDataTreeProducerOptimized::analyze(edm::Event const &event_obj,
                                    edm::EventSetup const &iSetup) {

    // Event info
    run = event_obj.id().run();
    lumi = event_obj.luminosityBlock();
    event = event_obj.id().event();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////  
    // Test Discriminant 
    //---------------------------- Jet CSV discriminantor -----------------------
    edm::Handle<reco::JetTagCollection> tagHandle_CSV;
    event_obj.getByLabel("combinedSecondaryVertexBJetTags", tagHandle_CSV);
    const reco::JetTagCollection & tag_CSV = *(tagHandle_CSV.product());
    
    //---------------------------- Jet JBP tag discriminantor -------------------
    edm::Handle<reco::JetTagCollection> tagHandle_JBP;
    event_obj.getByLabel("jetBProbabilityBJetTags", tagHandle_JBP); 
    const reco::JetTagCollection & tag_JBP = *(tagHandle_JBP.product());

    //---------------------------- Jet TCHP discriminator -----------------------
    edm::Handle<reco::JetTagCollection> tagHandle_TCHP;
    event_obj.getByLabel("trackCountingHighPurBJetTags", tagHandle_TCHP); 
    const reco::JetTagCollection & tag_TCHP = *(tagHandle_TCHP.product());
    //cout << "Found " << ip.size() << " TagInfo" << endl;

    
    
    // Print out the info
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    std::cout << "event " << event << std::endl;  
//    std::cout << "-----------------------------------------------------------------------" << std::endl;    
//    std::cout << "tag_CSV.size()    " << tag_CSV.size() << std::endl; 
//    std::cout << "-----------------------------------------------------------------------" << std::endl;    
//    std::cout << "tag_JBP.size()    " << tag_JBP.size() << std::endl; 
//
//    std::cout <<      "---------------------------- Jet CSV tag Info -------------------" << std::endl;  
//    for (int i = 0; i != (int)tag_CSV.size(); i++)
//     {
//     std::cout << "ptCSV   " << tag_CSV[i].first -> pt() << "    etaCSV   " << tag_CSV[i].first -> eta() << "   phiCSV   " << tag_CSV[i].first -> phi() << std::endl;   
//     } 
//    std::cout <<      "---------------------------- Jet JBP tag Info -------------------" << std::endl; 
//    for (int i = 0; i != (int)tag_JBP.size(); i++)
//    { 
//      std::cout << "ptJBP   " << tag_JBP[i].first -> pt() << "  etaJBP    " << tag_JBP[i].first -> eta() << "    phiJBP   " << tag_JBP[i].first -> phi() << std::endl;   
//    }
//  //////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    /////////////////////////////////////////////////////////////////////////////////////////////////////////  
   
    // Test Flavour
    //if (mIsMCarlo){
    //edm::Handle<reco::JetFlavourInfoMatchingCollection> theJetFlavourInfos;
    //event_obj.getByLabel(jetFlavourInfos_, theJetFlavourInfos );
    //}
    // Print out the info
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // for ( reco::JetFlavourInfoMatchingCollection::const_iterator j  = theJetFlavourInfos->begin();
    //                                  j != theJetFlavourInfos->end();
    //                                  ++j ) {
    //      std::cout << "-------------------- Jet Flavour Info --------------------" << std::endl;
    //      reco::JetFlavourInfo aInfo = (*j).second;
    //      // ----------------------- Hadrons -------------------------------
    //      std::cout << "                      Hadron-based flavour: " << aInfo.getHadronFlavour() << std::endl;
    // }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Triggers
    edm::Handle<edm::TriggerResults>   triggerResultsHandle_;
    event_obj.getByLabel(triggerResultsTag_, triggerResultsHandle_);

    // Sanity checks
    assert(triggerResultsHandle_.isValid() && "Error in getting TriggerResults from Event!");
    assert(triggerResultsHandle_->size() == hltConfig_.size() && "Size mismatch between triggerResultsHandle_ and hltConfig_");
    
    // Number of triggers to be saved
    ntrg = triggerIndex_.size();

    // Iterate only over the selected jet triggers
    for (unsigned itrig = 0; itrig < ntrg; itrig++) {

        // Trigger bit
        Bool_t isAccepted = triggerResultsHandle_->accept(triggerIndex_[itrig]);
        triggers[itrig] = isAccepted;

        // Trigger prescales are retrieved using the trigger name
        std::string trgName = hltConfig_.triggerName(triggerIndex_[itrig]);
        const std::pair< int, int > prescalePair(hltConfig_.prescaleValues(event_obj, iSetup, trgName));

        // Total prescale: PreL1*PreHLT 
        prescales[itrig] = prescalePair.first*prescalePair.second;   
    }    

    // Rho
    Handle< double > rho_handle;
    event_obj.getByLabel(mSrcPFRho, rho_handle);
    rho = *rho_handle;


    // Generator Info

    // Retrieve pthat and mcweight (only MC)
    pthat = 0;
    if (mIsMCarlo && mUseGenInfo) {
        Handle< GenEventInfoProduct > hEventInfo;
        event_obj.getByLabel("generator", hEventInfo);

        // Monte Carlo weight (NOT AVAILABLE FOR 2011 MC!!)
        //mcweight = hEventInfo->weight();
        
        // Pthat 
        if (hEventInfo->hasBinningValues()) {
            pthat = hEventInfo->binningValues()[0];
        }
    }

    // Generator-level jets
    ngen = 0;
    if (mIsMCarlo) {

        Handle< GenJetCollection > genjets;
        event_obj.getByLabel(mGenJetsName, genjets);
    
        // Index of the simulated jet
        int gen_index = 0; 

        for (GenJetCollection::const_iterator i_gen = genjets->begin(); i_gen != genjets->end(); i_gen++)  {

            // pT and rapidity selection
            if (i_gen->pt() > mMinGenPt && fabs(i_gen->y()) < mMaxY) {
                gen_pt[gen_index] = i_gen->pt();
                gen_eta[gen_index] = i_gen->eta();
                gen_phi[gen_index] = i_gen->phi();
                gen_E[gen_index] = i_gen->energy();
                gen_index++;
            }
        }

        // Number of generated jets in this event
        ngen = gen_index;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////  
    // Secondary Vertex Info
    edm::Handle<reco::SecondaryVertexTagInfoCollection> tagInfosHandle;
    event_obj.getByLabel(secondaryVertexTagInfos_, tagInfosHandle);
    //event_obj.getByLabel("secondaryVertexTagInfos", tagInfosHandle);
    const reco::SecondaryVertexTagInfoCollection & tagInfoColl = *(tagInfosHandle.product());
   
    // Print out the info
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for(reco::SecondaryVertexTagInfoCollection::const_iterator iter = tagInfoColl.begin(); iter != tagInfoColl.end(); ++iter) 
    {
      // if there are reconstructed vertices in this jet
                     nSVertex = iter->nVertices();  
                   // if(iter->nVertices() >=1 ) {
                   //  std::cout<<"found secondary vertex with a flight distance of " << iter->flightDistance(0).value() << " cm"<< std::endl;
                   // }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////  
    
    
    // Vertex Info
    Handle<reco::VertexCollection> recVtxs;
    event_obj.getByLabel(mOfflineVertices, recVtxs);


    // PF AK5 Jets

    edm::Handle<reco::PFJetCollection> ak5_handle;
    event_obj.getByLabel(mPFak5JetsName, ak5_handle);
    const JetCorrector* corrector_ak5 = JetCorrector::getJetCorrector(mJetCorr_ak5, iSetup);


    // Jet Track Association (JTA)
    edm::Handle <reco::TrackCollection> tracks_h;
    event_obj.getByLabel ("generalTracks", tracks_h);
    std::auto_ptr<reco::JetTracksAssociation::Container> tracksInJets (new reco::JetTracksAssociation::Container (reco::JetRefBaseProd(ak5_handle)));
    // format inputs
    std::vector <edm::RefToBase<reco::Jet> > allJets;
    allJets.reserve (ak5_handle->size());
    for (unsigned i = 0; i < ak5_handle->size(); ++i)
    {
      edm::RefToBase<reco::Jet> jetRef(edm::Ref<reco::PFJetCollection>(ak5_handle, i));
      allJets.push_back(jetRef);
    }
    std::vector <reco::TrackRef> allTracks;
    allTracks.reserve(tracks_h->size());
    for (unsigned i = 0; i < tracks_h->size(); ++i) 
      allTracks.push_back (reco::TrackRef(tracks_h, i));
    // run JTA algorithm
    JetTracksAssociationDRVertex mAssociator(0.5); // passed argument: 0.5 cone size
    mAssociator.produce (&*tracksInJets, allJets, allTracks);
  
    // Index of the selected jet 
    int ak5_index = 0;

    // Jet energy correction factor
    double jec = -1.0;

    // Jets will be unsorted in pT after applying JEC,  
    // therefore store corrected jets in a new collection (map): key (double) is pT * -1 (key), 
    // value (std::pair<PFJet*, double>) is pair of original jet iterator and corresponding JEC factor
    std::map<double, std::pair<reco::PFJetCollection::const_iterator, double> > sortedJets;
    for (auto i_ak5jet_orig = ak5_handle->begin(); i_ak5jet_orig != ak5_handle->end(); ++i_ak5jet_orig) {
        
        /////////////////////////////////////////////////////////////////////////////////////////////
        // Para chequear la correccion 
        /////////////////////////////////////////////////////////////////////////////////////////////
        printf( "\nmi jet sin corregir\n");
        printf("  pt = %f\n",                           i_ak5jet_orig->pt());
        printf("  chargedHadronEnergyFraction  = %f\n", i_ak5jet_orig->chargedHadronEnergyFraction());
        printf("  muonEnergyFraction  = %f\n",          i_ak5jet_orig->muonEnergyFraction());
        printf("  neutralHadronMultiplicity = %i\n",    i_ak5jet_orig->neutralHadronMultiplicity());
        printf("  chargedHadronMultiplicity = %i\n",    i_ak5jet_orig->chargedHadronMultiplicity());
        printf("  chargedMultiplicity       = %i\n",    i_ak5jet_orig->chargedMultiplicity());
        printf("  neutralMultiplicity       = %i\n",    i_ak5jet_orig->chargedHadronMultiplicity());
        ////////////////////////////////////////////////////////////////////////////////////////////

        // take jet energy correction and get corrected pT
        jec = corrector_ak5->correction(*i_ak5jet_orig, event_obj, iSetup);
        // Multiply pT by -1 in order to have largest pT jet first (sorted in ascending order by default)
        sortedJets.insert(std::pair<double, std::pair<reco::PFJetCollection::const_iterator, double> >(-1 * i_ak5jet_orig->pt() * jec, std::pair<reco::PFJetCollection::const_iterator, double>(i_ak5jet_orig, jec)));
    }

    // Iterate over the jets (sorted in pT) of the event
    for (auto i_ak5jet_orig = sortedJets.begin(); i_ak5jet_orig != sortedJets.end(); ++i_ak5jet_orig) {

        // Apply jet energy correction "on the fly":
        // copy original (uncorrected) jet;
        PFJet corjet = *((i_ak5jet_orig->second).first);
        // take stored JEC factor
        jec = (i_ak5jet_orig->second).second;
        // apply JEC
        corjet.scaleEnergy(jec);
        // pointer for further use
        const PFJet* i_ak5jet = &corjet;

        /////////////////////////////////////////////////////////////////////////////////////////////
        // Para chequear la correccion 
        /////////////////////////////////////////////////////////////////////////////////////////////
        printf( "\nmi jet  corregido\n");
        printf("  pt = %f\n",i_ak5jet->pt());
        printf("  chargedHadronEnergyFraction  = %f\n", i_ak5jet->chargedHadronEnergyFraction());
        printf("  muonEnergyFraction  = %f\n",          i_ak5jet->muonEnergyFraction());
        printf("  neutralHadronMultiplicity = %i\n",    i_ak5jet->neutralHadronMultiplicity());
        printf("  chargedHadronMultiplicity = %i\n",    i_ak5jet->chargedHadronMultiplicity());
        printf("  chargedMultiplicity       = %i\n",    i_ak5jet->chargedMultiplicity());
        printf("  neutralMultiplicity       = %i\n",    i_ak5jet->chargedHadronMultiplicity());

        // Skip the current iteration if jet is not selected
        if (fabs(i_ak5jet->y()) > mMaxY || 
            (i_ak5jet->pt()) < mMinPFPt) {
            continue;
        }

        // Computing beta and beta*

        // Get tracks
        reco::TrackRefVector tracks = reco::JetTracksAssociation::getValue(*tracksInJets, *((i_ak5jet_orig->second).first));

        float sumTrkPt(0.0), sumTrkPtBeta(0.0),sumTrkPtBetaStar(0.0);
        beta[ak5_index] = 0.0;
        bstar[ak5_index] = 0.0;


        //Test track properties here
        //////////////////////////////////////////////////////////////////////////
        // track multiplicity in the jet
          ntracks[ak5_index] = 0.0;
        // Index of the track in the jet
          int track_index = 0;
 
        // Loop over tracks of the jet
        for(auto i_trk = tracks.begin(); i_trk != tracks.end(); i_trk++) {

            if (recVtxs->size() == 0) break;

            track_pt [track_index] = (*i_trk)->pt();           
             // Hit pattern of the track
             const reco::HitPattern& p = (*i_trk)->hitPattern();
             // Loop over the hits of the track
             for (int i=0; i<p.numberOfHits(); i++) {
              track_nValidPixelHits[track_index] = p.numberOfValidPixelHits() ;
              track_nValidTrackerHits [track_index] = p.numberOfValidTrackerHits() ;
             }
             // Extract the Impact paramenter info for this track
             
             Handle<TrackIPTagInfoCollection> ipHandle;
             event_obj.getByLabel(m_ipassoc, ipHandle);
             const TrackIPTagInfoCollection & ip = *(ipHandle.product());
             TrackIPTagInfoCollection::const_iterator it = ip.begin();
             for(; it != ip.end(); it++)
              { 
                TrackIPTagInfo::TrackIPData data = it->impactParameterData()[track_index];  
             /*   cout << selTracks[j]->pt() << "\t";
                cout << it->probabilities(0)[j] << "\t";
                cout << it->probabilities(1)[j] << "\t";
                cout << data.ip3d.value() << "\t";
                cout << data.ip3d.significance() << "\t";
                cout << data.distanceToJetAxis.value() << "\t";
                cout << data.distanceToJetAxis.significance() << "\t";
                cout << data.distanceToGhostTrack.value() << "\t";
                cout << data.distanceToGhostTrack.significance() << "\t";
                cout << data.closestToJetAxis << "\t";
                cout << (data.closestToJetAxis - pv).mag() << "\t";
                cout << data.closestToGhostTrack << "\t";
                cout << (data.closestToGhostTrack - pv).mag() << "\t";
                cout << data.ip2d.value() << "\t";
                cout << data.ip2d.significance() <<  endl;     
           */   }
           
            // Sum pT
            sumTrkPt += (*i_trk)->pt();
            
            // Loop over vertices
            for (unsigned ivtx = 0; ivtx < recVtxs->size(); ivtx++) {
                reco::Vertex vertex = (*recVtxs)[ivtx];

                // Loop over tracks associated with the vertex
                bool flagBreak = false;
                if (!(vertex.isFake()) && 
                    vertex.ndof() >= mGoodVtxNdof && 
                    fabs(vertex.z()) <= mGoodVtxZ) {
                    
                    for(auto i_vtxTrk = vertex.tracks_begin(); i_vtxTrk != vertex.tracks_end(); ++i_vtxTrk) {
                        
                        // Match the jet track to the track from the vertex
                        reco::TrackRef trkRef(i_vtxTrk->castTo<reco::TrackRef>());
                        
                        // Check for matching vertices
                        if (trkRef == (*i_trk)) {
                            if (ivtx == 0) {
                                sumTrkPtBeta += (*i_trk)->pt();
                            }
                            else {
                                sumTrkPtBetaStar += (*i_trk)->pt();
                            }
                            flagBreak = true;
                            break;
                        } 
                    } 
                    if(flagBreak)
                      break;
                } 
            } 
        track_index ++; 
        }
   
       // Number of tracks in the jet 
       ntrack = track_index;
       ntracks[ak5_index] = ntrack; 

    
        if (sumTrkPt > 0) {
            beta[ak5_index]   = sumTrkPtBeta/sumTrkPt;
            bstar[ak5_index]  = sumTrkPtBetaStar/sumTrkPt;
        } 


        // Jet composition
        // (all energy fractions have to be multiplied by the JEC factor)
        chf[ak5_index]     = i_ak5jet->chargedHadronEnergyFraction() * jec;
        nhf[ak5_index]     = (i_ak5jet->neutralHadronEnergyFraction() + i_ak5jet->HFHadronEnergyFraction()) * jec;
        phf[ak5_index]     = i_ak5jet->photonEnergyFraction() * jec;
        elf[ak5_index]     = i_ak5jet->electronEnergyFraction() * jec;
        muf[ak5_index]     = i_ak5jet->muonEnergyFraction() * jec;
        hf_hf[ak5_index]   = i_ak5jet->HFHadronEnergyFraction() * jec;
        hf_phf[ak5_index]  = i_ak5jet->HFEMEnergyFraction() * jec;
        hf_hm[ak5_index]   = i_ak5jet->HFHadronMultiplicity();
        hf_phm[ak5_index]  = i_ak5jet->HFEMMultiplicity();
        chm[ak5_index]     = i_ak5jet->chargedHadronMultiplicity();
        nhm[ak5_index]     = i_ak5jet->neutralHadronMultiplicity();
        phm[ak5_index]     = i_ak5jet->photonMultiplicity();
        elm[ak5_index]     = i_ak5jet->electronMultiplicity();
        mum[ak5_index]     = i_ak5jet->muonMultiplicity();
    
        int npr      = i_ak5jet->chargedMultiplicity() + i_ak5jet->neutralMultiplicity();

        bool isHighEta = fabs(i_ak5jet->eta()) > 2.4;
        bool isLowEta = fabs(i_ak5jet->eta()) <= 2.4 && 
                        nhf[ak5_index] < 0.9 &&
                        phf[ak5_index] < 0.9 && 
                        elf[ak5_index] < 0.99 && 
                        chf[ak5_index] > 0 && 
                        chm[ak5_index] > 0;
        bool tightID =  npr > 1 && 
                        phf[ak5_index] < 0.99 && 
                        nhf[ak5_index] < 0.99 &&
                        (isLowEta || isHighEta);

        // Jet ID requirement for btagging commisioning plots 2011 collected from twiki: 
        // https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetID#Recommendations_for_7_TeV_data_a
        nhfJet[ak5_index]   = i_ak5jet->neutralHadronEnergyFraction() * jec; 
        nemfJet[ak5_index]  = i_ak5jet->neutralEmEnergyFraction() * jec;
        chemfJet[ak5_index] = i_ak5jet->chargedEmEnergyFraction() * jec;
        chmJet[ak5_index]   = i_ak5jet->chargedMultiplicity (); 
        // Loose WP
        bool looseID = i_ak5jet->pt() > 10.0 && fabs(i_ak5jet->eta()) < 2.4 && chf[ak5_index] > 0.0 &&  nhfJet[ak5_index] < 0.99 && chmJet[ak5_index] > 0.0 && nemfJet[ak5_index] < 0.99 &&  chemfJet[ak5_index] < 0.99 && npr > 1; 
        jet_looseID[ak5_index] = looseID; 
        
 
        // Variables of the tuple
        jet_tightID[ak5_index] = tightID;
        jet_area[ak5_index] = i_ak5jet->jetArea();
        jet_jes[ak5_index] = jec; // JEC factor

        // p4 is already corrected!
        auto p4 = i_ak5jet->p4();
        jet_pt[ak5_index]   = p4.Pt();
        jet_eta[ak5_index]  = p4.Eta();
        jet_phi[ak5_index]  = p4.Phi();
        jet_E[ak5_index]    = p4.E(); 
        
        // Matching a GenJet to this PFjet
        jet_igen[ak5_index] = 0;
        if (mIsMCarlo && ngen > 0) {

            // Index of the generated jet matching this PFjet
            jet_igen[ak5_index] = -1; // is -1 if no matching jet

            // Search generated jet with minimum distance to this PFjet   
            float r2min(999);
            for (unsigned int gen_index = 0; gen_index != ngen; gen_index++) {
                double deltaR2 = reco::deltaR2( jet_eta[ak5_index], 
                                                jet_phi[ak5_index],
                                                gen_eta[gen_index], 
                                                gen_phi[gen_index]);
                if (deltaR2 < r2min) {
                    r2min = deltaR2;
                    jet_igen[ak5_index] = gen_index;
                }
            }
        }
       
    //Test Flavour
    if (mIsMCarlo){
      
      edm::Handle<reco::JetFlavourInfoMatchingCollection> theJetFlavourInfos;
      event_obj.getByLabel(jetFlavourInfos_, theJetFlavourInfos );
      
      HadronF[ak5_index] = -999; PartonF[ak5_index] = -999; nBHadrons [ak5_index] = -999;
      for ( reco::JetFlavourInfoMatchingCollection::const_iterator j  = theJetFlavourInfos->begin();
                                     j != theJetFlavourInfos->end();
                                     ++j ) {

          const reco::Jet *aJet = (*j).first.get();
          double deltaR2 = reco::deltaR2( jet_eta[ak5_index],
                                          jet_phi[ak5_index],
                                          aJet->eta(),
                                          aJet->phi()); 

 
         //---------------------------- Jet Flavour Info -------------------
         if ( deltaR2 < 0.01) // check the value of the deltaR2
         {
            reco::JetFlavourInfo aInfo = (*j).second;
            // ----------------------- Hadrons-based flavour -------------------------------
            HadronF[ak5_index] = aInfo.getHadronFlavour();
            // ----------------------- Parton-based flavour  -------------------------------
            PartonF[ak5_index] = aInfo.getPartonFlavour();
            //------------------------ # of clustered b-hadrons ----------------------------
            const reco::GenParticleRefVector & bHadrons = aInfo.getbHadrons();

            nBHadrons [ak5_index] = bHadrons.size();          
            std::cout << " nBHadrons " << ak5_index << " =  " << nBHadrons [ak5_index]  << std::endl;  

            /*if (bHadrons.size()== 2){std::cout << "                      # of clustered b hadrons: " << bHadrons.size() << std::endl;
            nBHadrons [ak5_index] = bHadrons.size();          
            }
            *///Checking info
               etaF [ak5_index] = aJet->eta();
               phiF [ak5_index] = aJet->phi();
          
            //break;
          }
      }
    }
    // Test Discriminant 
    // Get the discriminant info for the ak5 selected jet
    //---------------------------- Jet CSV tag Info -------------------
 //   edm::Handle<reco::JetTagCollection> tagHandle_CSV;
 //   event_obj.getByLabel("combinedSecondaryVertexBJetTags", tagHandle_CSV);
 //   const reco::JetTagCollection & tag_CSV = *(tagHandle_CSV.product());
 //   
 //   //---------------------------- Jet JBP tag Info -------------------
 //   edm::Handle<reco::JetTagCollection> tagHandle_JBP;
 //   event_obj.getByLabel("jetBProbabilityBJetTags", tagHandle_JBP); 
 //   const reco::JetTagCollection & tag_JBP = *(tagHandle_JBP.product()); 

    // Loop over JetTagCollection  matching with ak5 selected jets
    // Index of the tagged jet collection matching this PFjet 
    jet_CSV[ak5_index]  = -999;// is -1 if no matching jet
    jet_JBP [ak5_index] = -999;// is -1 if no matching jet
    jet_TCHP[ak5_index] = -999;// is -1 if no matching jet 
    //for (int i = 0; i != 10; i++)
    for (int i = 0; i != (int)tag_CSV.size(); i++)
     {
        double deltaR2_Tag1 = reco::deltaR2( jet_eta[ak5_index],
                                             jet_phi[ak5_index],
                                             tag_CSV[i].first -> eta(),
                                             tag_CSV[i].first -> phi() 
                                           );
      if ( (deltaR2_Tag1) < 0.01) // check the value of the deltaR2 
      {
        jet_CSV [ak5_index] = tag_CSV[i].second;
        jet_JBP [ak5_index] = tag_JBP[i].second;
        jet_TCHP[ak5_index] = tag_TCHP[i].second;
       // std::cout << "---------------------Matching--------------------------------------------------" << std::endl;    
     //std::cout << "ptCSV   " << tag_CSV[i].first -> pt() << "    etaCSV   " << tag_CSV[i].first -> eta() << "   phiCSV   " << tag_CSV[i].first -> phi() << std::endl;   
        //std::cout << "-----------------------------------------------------------------------" << std::endl;    
        break;
       }

    } 

    //---------------------------- Jet JBP tag Info -------------------
  /*  edm::Handle<reco::JetTagCollection> tagHandle_JBP;
    event_obj.getByLabel("jetBProbabilityBJetTags", tagHandle_JBP); 
    const reco::JetTagCollection & tag_JBP = *(tagHandle_JBP.product()); 
    //for (int i = 0; i != 10; i++)
    for (int i = 0; i != (int)tag_JBP.size(); i++)
    { 
      double deltaR2_Tag2 = reco::deltaR2( jet_eta[ak5_index],
                                             jet_phi[ak5_index],
                                             tag_JBP[i].first -> eta(),
                                             tag_JBP[i].first -> phi()
                                           );
       if ( abs(deltaR2_Tag2) < 0.01)
      {
      jet_JBP [ak5_index] = tag_JBP[i].second;
      break;
      }

    }
  */
   //This does not work 
   //const reco::JetTagCollection & tagColl_CSV = *(tagHandle_CSV.product());
  // for (JetTagCollection::const_iterator tagI = tagColl_CSV.begin();
  //           tagI != tagColl_CSV.end(); ++tagI)
  //  
  //  { double etaT = tagI->first()->eta(); double phiT = tagI->first()->phi();
  //   double deltaR2_Tag1 = reco::deltaR2( jet_eta[ak5_index],
  //                                        jet_phi[ak5_index],
  //                                        etaT,
  //                                        phiT);
  //  //---------------------------- Jet CSV tag Info -------------------
  //    if ( deltaR2_Tag1 < 0.1)
  //    {
  //     jet_CSV [ak5_index] = tagI->second;
  //     break;
  //    }
  //  }

   // Test JBP discriminant 
 //  const reco::JetTagCollection & tagColl_JBP = *(tagHandle_JBP.product());
 //  for (JetTagCollection::const_iterator tagII = tagColl_JBP.begin();
 //            tagII != tagColl_JBP.end(); ++tagII)
 //   {
 //   double deltaR2_Tag2 = reco::deltaR2( jet_eta[ak5_index],
 //                                         jet_phi[ak5_index],
 //                                         tagII->first()->eta(),
 //                                         tagII->first()->phi());   

 //   //---------------------------- Jet CSV tag Info -------------------
 //   if ( deltaR2_Tag2 < 0.1)
 //     {
 //      jet_JBP [ak5_index] = tagII->second;
 //     }
 //   }


    ak5_index++;
    }  

    // Number of selected jets in the event
    njet = ak5_index;    


    // Four leading AK7 Jets

    edm::Handle<reco::PFJetCollection> ak7_handle;
    event_obj.getByLabel(mPFak7JetsName, ak7_handle);
    const JetCorrector* corrector_ak7 = JetCorrector::getJetCorrector(mJetCorr_ak7, iSetup);

    // Index of the selected jet 
    int ak7_index = 0;

    // Jets will be unsorted in pT after applying JEC,  
    // therefore store corrected jets in a new collection (map): key (double) is pT * -1 (key), 
    // value (std::pair<PFJet*, double>) is pair of original jet iterator and corresponding JEC factor
    sortedJets.clear();
    for (auto i_ak7jet_orig = ak7_handle->begin(); i_ak7jet_orig != ak7_handle->end(); ++i_ak7jet_orig) {
        // take jet energy correction and get corrected pT
        jec = corrector_ak7->correction(*i_ak7jet_orig, event_obj, iSetup);
        // Multiply pT by -1 in order to have largest pT jet first (sorted in ascending order by default)
        sortedJets.insert(std::pair<double, std::pair<reco::PFJetCollection::const_iterator, double> >(-1 * i_ak7jet_orig->pt() * jec, std::pair<reco::PFJetCollection::const_iterator, double>(i_ak7jet_orig, jec)));
    }

    // Iterate over the jets (sorted in pT) of the event
    for (auto i_ak7jet_orig = sortedJets.begin(); i_ak7jet_orig != sortedJets.end() && ak7_index < 4; ++i_ak7jet_orig) {

        // Apply jet energy correction "on the fly":
        // copy original (uncorrected) jet;
        PFJet corjet = *((i_ak7jet_orig->second).first);
        // take stored JEC factor
        jec = (i_ak7jet_orig->second).second;
        // apply JEC
        corjet.scaleEnergy(jec);
        // pointer for further use
        const PFJet* i_ak7jet = &corjet;

        // Skip the current iteration if jet is not selected
        if (fabs(i_ak7jet->y()) > mMaxY || 
            (i_ak7jet->pt()) < mMinPFPt) {
            continue;
        }

        // Variables of the tuple
        jet_area_ak7[ak7_index] = i_ak7jet->jetArea();
        jet_jes_ak7[ak7_index] = jec; // JEC factor

        // p4 is already corrected!
        auto p4 = i_ak7jet->p4();
        jet_pt_ak7[ak7_index]   = p4.Pt();
        jet_eta_ak7[ak7_index]  = p4.Eta();
        jet_phi_ak7[ak7_index]  = p4.Phi();
        jet_E_ak7[ak7_index]    = p4.E(); 
        
        // Matching AK5 jet to this AK7 jet
        // Index of the generated jet matching this PFjet
        ak7_to_ak5[ak7_index] = -1; // -1 if no matching jet

        float r2min(999);
        for (unsigned int ak5_index = 0; ak5_index != njet; ak5_index++) {

            // Compute distance squared
            double deltaR2 = reco::deltaR2( jet_eta_ak7[ak7_index], 
                                            jet_phi_ak7[ak7_index],
                                            jet_eta[ak5_index], 
                                            jet_phi[ak5_index]);
            if (deltaR2 < r2min) {
                r2min = deltaR2;
                ak7_to_ak5[ak7_index] = ak5_index;
            }
        }
        
    ak7_index++;
    }  
    // Number of saved jets in the event
    njet_ak7 = ak7_index;    


    // MET
    Handle< PFMETCollection > met_handle;
    event_obj.getByLabel("pfMet", met_handle);

    met = (*met_handle)[0].et();
    sumet = (*met_handle)[0].sumEt();

    // Finally, fill the tree
    if (njet >= (unsigned)mMinNPFJets && 
        njet_ak7 >= (unsigned)mMinNPFJets ) {            
            mTree->Fill();
    }
}


void OpenDataTreeProducerOptimized::endRun(edm::Run const &iRun, edm::EventSetup const &iSetup) {

}

OpenDataTreeProducerOptimized::~OpenDataTreeProducerOptimized() {
}


DEFINE_FWK_MODULE(OpenDataTreeProducerOptimized);
