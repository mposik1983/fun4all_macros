#pragma once

#include "GlobalVariables.C"

#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <g4detectors/PHG4CylinderSubsystem.h>
#include <g4eval/PHG4DstCompressReco.h>
#include <g4main/HepMCNodeReader.h>
#include <g4main/PHG4Reco.h>
#include <g4main/PHG4TruthSubsystem.h>
#include <phfield/PHFieldConfig.h>
#include <g4decayer/EDecayType.hh>
#include "G4_Aerogel.C"
#include "G4_BlackHole.C"
#include "G4_CEmc_EIC.C"
#include "G4_DIRC.C"
#include "G4_EEMC.C"
#include "G4_FEMC_EIC.C"
#include "G4_FHCAL.C"
#include "G4_GEM_EIC.C"
#include "G4_HcalIn_ref.C"
#include "G4_HcalOut_ref.C"
#include "G4_Magnet.C"
#include "G4_Mvtx.C"
#include "G4_PSTOF.C"
#include "G4_Pipe_EIC.C"
#include "G4_PlugDoor_EIC.C"
#include "G4_RICH.C"
#include "G4_TPC_EIC.C"
#include "G4_Tracking_EIC.C"
#include "G4_User.C"
#include "G4_WorldSize.C"

R__LOAD_LIBRARY(libg4decayer.so)
R__LOAD_LIBRARY(libg4detectors.so)

void RunLoadTest() {}

void G4Init()
{
  // load detector/material macros and execute Init() function

  if (Enable::PIPE)
  {
    PipeInit();
  }

  if (Enable::PLUGDOOR)
  {
    PlugDoorInit();
  }

  if (Enable::EGEM)
  {
    EGEM_Init();
  }

  if (Enable::FGEM)
  {
    FGEM_Init();
  }
  if (Enable::MVTX)
  {
    MvtxInit();
  }
  if (Enable::TPC)
  {
    TPCInit();
  }
  if (Enable::TRACKING)
  {
    TrackingInit();
  }

  if (Enable::CEMC)
  {
    CEmcInit(72);  // make it 2*2*2*3*3 so we can try other combinations
  }

  if (Enable::HCALIN)
  {
    HCalInnerInit(1);
  }

  if (Enable::MAGNET)
  {
    MagnetInit();
  }
  if (Enable::HCALOUT)
  {
    HCalOuterInit();
  }

  if (Enable::FEMC)
  {
    FEMCInit();
  }

  if (Enable::FHCAL)
  {
    FHCALInit();
  }

  if (Enable::EEMC)
  {
    EEMCInit();
  }

  if (Enable::DIRC)
  {
    DIRCInit();
  }

  if (Enable::RICH)
  {
    RICHInit();
  }

  if (Enable::AEROGEL)
  {
    AerogelInit();
  }
  if (Enable::USER)
  {
    UserInit();
  }

  if (Enable::BLACKHOLE)
  {
    BlackHoleInit();
  }
}

int G4Setup(const int absorberactive = 0,
            const string &field = "1.5",
            const EDecayType decayType = EDecayType::kAll,
            const float magfield_rescale = 1.0)
{
  //---------------
  // Load libraries
  //---------------

  gSystem->Load("libg4detectors.so");
  gSystem->Load("libg4testbench.so");

  //---------------
  // Fun4All server
  //---------------

  Fun4AllServer *se = Fun4AllServer::instance();

  // read-in HepMC events to Geant4 if there is any
  HepMCNodeReader *hr = new HepMCNodeReader();
  se->registerSubsystem(hr);

  PHG4Reco *g4Reco = new PHG4Reco();
  g4Reco->set_rapidity_coverage(1.1);  // according to drawings
                                       // uncomment to set QGSP_BERT_HP physics list for productions
                                       // (default is QGSP_BERT for speed)
  //  g4Reco->SetPhysicsList("QGSP_BERT_HP");

  if (decayType != EDecayType::kAll)
  {
    g4Reco->set_force_decay(decayType);
  }

  double fieldstrength;
  istringstream stringline(field);
  stringline >> fieldstrength;
  if (stringline.fail())
  {  // conversion to double fails -> we have a string

    if (field.find("sPHENIX.root") != string::npos)
    {
      g4Reco->set_field_map(field, PHFieldConfig::Field3DCartesian);
    }
    else
    {
      g4Reco->set_field_map(field, PHFieldConfig::kField2D);
    }
  }
  else
  {
    g4Reco->set_field(fieldstrength);  // use const soleniodal field
  }
  g4Reco->set_field_rescale(magfield_rescale);

  double radius = 0.;

  //----------------------------------------
  // PIPE
  if (Enable::PIPE)
  {
    radius = Pipe(g4Reco, radius, absorberactive);
  }
  //----------------------------------------

  if (Enable::EGEM)
  {
    EGEMSetup(g4Reco);
  }

  if (Enable::FGEM)
  {
    FGEMSetup(g4Reco);
  }

  if (Enable::MVTX)
  {
    radius = Mvtx(g4Reco, radius, absorberactive);
  }
  if (Enable::TPC)
  {
    radius = TPC(g4Reco, radius, absorberactive);
  }
  if (Enable::TRACKING)
  {
    //    radius = Tracking(g4Reco, radius, absorberactive);
  }
  //----------------------------------------
  // CEMC
  //
  if (Enable::CEMC)
  {
    radius = CEmc(g4Reco, radius, 8, absorberactive);
  }
  //  if (do_cemc) radius = CEmc_Vis(g4Reco, radius, 8, absorberactive);// for visualization substructure of SPACAL, slow to render

  //----------------------------------------
  // HCALIN

  if (Enable::HCALIN)
  {
    radius = HCalInner(g4Reco, radius, 4, absorberactive);
  }
  //----------------------------------------
  // MAGNET

  if (Enable::MAGNET)
  {
    radius = Magnet(g4Reco, radius, 0, absorberactive);
  }
  //----------------------------------------
  // HCALOUT

  if (Enable::HCALOUT)
  {
    radius = HCalOuter(g4Reco, radius, 4, absorberactive);
  }
  //----------------------------------------
  // FEMC

  if (Enable::FEMC)
  {
    FEMCSetup(g4Reco, absorberactive);
  }

  //----------------------------------------
  // FHCAL

  if (Enable::FHCAL)
  {
    FHCALSetup(g4Reco, absorberactive);
  }
  //----------------------------------------
  // EEMC

  if (Enable::EEMC)
  {
    EEMCSetup(g4Reco, absorberactive);
  }

  //----------------------------------------
  // PID

  if (Enable::DIRC)
  {
    DIRCSetup(g4Reco);
  }

  if (Enable::RICH)
  {
    RICHSetup(g4Reco);
  }

  if (Enable::AEROGEL)
  {
    AerogelSetup(g4Reco);
  }
  //----------------------------------------
  // sPHENIX forward flux return door
  if (Enable::PLUGDOOR)
  {
    PlugDoor(g4Reco, absorberactive);
  }
  if (Enable::USER)
  {
    UserDetector(g4Reco);
  }
  //----------------------------------------
  // BLACKHOLE if enabled, needs info from all previous sub detectors for dimensions
  if (Enable::BLACKHOLE)
  {
    BlackHole(g4Reco, radius);
  }

  PHG4TruthSubsystem *truth = new PHG4TruthSubsystem();
  g4Reco->registerSubsystem(truth);
  // finally adjust the world size in case the default is too small
  WorldSize(g4Reco, radius);

  se->registerSubsystem(g4Reco);
  return 0;
}

void ShowerCompress(int verbosity = 0)
{
  gSystem->Load("libfun4all.so");
  gSystem->Load("libg4eval.so");

  Fun4AllServer *se = Fun4AllServer::instance();

  PHG4DstCompressReco *compress = new PHG4DstCompressReco("PHG4DstCompressReco");
  compress->AddHitContainer("G4HIT_PIPE");
  compress->AddHitContainer("G4HIT_SVTXSUPPORT");
  compress->AddHitContainer("G4HIT_CEMC_ELECTRONICS");
  compress->AddHitContainer("G4HIT_CEMC");
  compress->AddHitContainer("G4HIT_ABSORBER_CEMC");
  compress->AddHitContainer("G4HIT_CEMC_SPT");
  compress->AddHitContainer("G4HIT_ABSORBER_HCALIN");
  compress->AddHitContainer("G4HIT_HCALIN");
  compress->AddHitContainer("G4HIT_HCALIN_SPT");
  compress->AddHitContainer("G4HIT_MAGNET");
  compress->AddHitContainer("G4HIT_ABSORBER_HCALOUT");
  compress->AddHitContainer("G4HIT_HCALOUT");
  compress->AddHitContainer("G4HIT_BH_1");
  compress->AddHitContainer("G4HIT_BH_FORWARD_PLUS");
  compress->AddHitContainer("G4HIT_BH_FORWARD_NEG");
  compress->AddCellContainer("G4CELL_CEMC");
  compress->AddCellContainer("G4CELL_HCALIN");
  compress->AddCellContainer("G4CELL_HCALOUT");
  compress->AddTowerContainer("TOWER_SIM_CEMC");
  compress->AddTowerContainer("TOWER_RAW_CEMC");
  compress->AddTowerContainer("TOWER_CALIB_CEMC");
  compress->AddTowerContainer("TOWER_SIM_HCALIN");
  compress->AddTowerContainer("TOWER_RAW_HCALIN");
  compress->AddTowerContainer("TOWER_CALIB_HCALIN");
  compress->AddTowerContainer("TOWER_SIM_HCALOUT");
  compress->AddTowerContainer("TOWER_RAW_HCALOUT");
  compress->AddTowerContainer("TOWER_CALIB_HCALOUT");

  compress->AddHitContainer("G4HIT_FEMC");
  compress->AddHitContainer("G4HIT_ABSORBER_FEMC");
  compress->AddHitContainer("G4HIT_FHCAL");
  compress->AddHitContainer("G4HIT_ABSORBER_FHCAL");
  compress->AddCellContainer("G4CELL_FEMC");
  compress->AddCellContainer("G4CELL_FHCAL");
  compress->AddTowerContainer("TOWER_SIM_FEMC");
  compress->AddTowerContainer("TOWER_RAW_FEMC");
  compress->AddTowerContainer("TOWER_CALIB_FEMC");
  compress->AddTowerContainer("TOWER_SIM_FHCAL");
  compress->AddTowerContainer("TOWER_RAW_FHCAL");
  compress->AddTowerContainer("TOWER_CALIB_FHCAL");

  compress->AddHitContainer("G4HIT_EEMC");
  compress->AddHitContainer("G4HIT_ABSORBER_EEMC");
  compress->AddCellContainer("G4CELL_EEMC");
  compress->AddTowerContainer("TOWER_SIM_EEMC");
  compress->AddTowerContainer("TOWER_RAW_EEMC");
  compress->AddTowerContainer("TOWER_CALIB_EEMC");

  se->registerSubsystem(compress);

  return;
}

void DstCompress(Fun4AllDstOutputManager *out)
{
  if (out)
  {
    out->StripNode("G4HIT_PIPE");
    out->StripNode("G4HIT_SVTXSUPPORT");
    out->StripNode("G4HIT_CEMC_ELECTRONICS");
    out->StripNode("G4HIT_CEMC");
    out->StripNode("G4HIT_ABSORBER_CEMC");
    out->StripNode("G4HIT_CEMC_SPT");
    out->StripNode("G4HIT_ABSORBER_HCALIN");
    out->StripNode("G4HIT_HCALIN");
    out->StripNode("G4HIT_HCALIN_SPT");
    out->StripNode("G4HIT_MAGNET");
    out->StripNode("G4HIT_ABSORBER_HCALOUT");
    out->StripNode("G4HIT_HCALOUT");
    out->StripNode("G4HIT_BH_1");
    out->StripNode("G4HIT_BH_FORWARD_PLUS");
    out->StripNode("G4HIT_BH_FORWARD_NEG");
    out->StripNode("G4CELL_CEMC");
    out->StripNode("G4CELL_HCALIN");
    out->StripNode("G4CELL_HCALOUT");

    out->StripNode("G4HIT_FEMC");
    out->StripNode("G4HIT_ABSORBER_FEMC");
    out->StripNode("G4HIT_FHCAL");
    out->StripNode("G4HIT_ABSORBER_FHCAL");
    out->StripNode("G4CELL_FEMC");
    out->StripNode("G4CELL_FHCAL");

    out->StripNode("G4HIT_EEMC");
    out->StripNode("G4HIT_ABSORBER_EEMC");
    out->StripNode("G4CELL_EEMC");
  }
}
