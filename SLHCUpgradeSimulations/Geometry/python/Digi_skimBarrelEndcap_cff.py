import FWCore.ParameterSet.Config as cms
# Mostly copied from Configuration/StandardSequences/python/Digi_cff.py
# In the future we will split off parts into a PixelDigi_Phase1_cff
#                                                    
# Full-scale Digitization of the simulated hits      
# in all CMS subdets : Tracker, ECAL, HCAl, Muon's;  
# MixingModule (at least in zero-pileup mode) needs  
# to be included to make Digi's operational, since   
# it's required for ECAL/HCAL & Muon's                
# Defined in a separate fragment
#                                                    
# Tracker Digis (Pixel + SiStrips)
# returns sequence "trDigi"
#
from SimTracker.Configuration.SimTracker_cff import *
# Calorimetry Digis (Ecal + Hcal) - * unsuppressed *
# returns sequence "calDigi"
from SimCalorimetry.Configuration.SimCalorimetry_cff import *
# Muon Digis (CSC + DT + RPC)
# returns sequence "muonDigi"
#
from SimMuon.Configuration.SimMuon_cff import *
#
# include TrackingParticle Producer
# NOTA BENE: it MUST be run here at the moment, since it depends 
# of the availability of the CrossingFrame in the Event
#
from SimGeneral.Configuration.SimGeneral_cff import *
#
# Phase 1 Modifications
#
#from SimTracker.SiPixelDigitizer.PixelDigi_cfi import *
simSiPixelDigis.MissCalibrate = False
simSiPixelDigis.LorentzAngle_DB = False
simSiPixelDigis.killModules = False
simSiPixelDigis.useDB = False
simSiPixelDigis.DeadModules_DB = False
simSiPixelDigis.NumPixelBarrel = cms.int32(10)
simSiPixelDigis.NumPixelEndcap = cms.int32(10)
simSiPixelDigis.AddPixelInefficiency = -1

# FULL STUFF
#doAllDigi = cms.Sequence(trDigi+calDigi+muonDigi)
#pdigi = cms.Sequence(cms.SequencePlaceholder("randomEngineStateProducer")*cms.SequencePlaceholder("mix")*doAllDigi*trackingParticles)
#pdigi.remove(simCastorDigis)

# TRACKER ONLY
doAllDigi = cms.Sequence(trDigi)
pdigi = cms.Sequence(cms.SequencePlaceholder("randomEngineStateProducer")*cms.SequencePlaceholder("mix")*doAllDigi*trackingParticles*addPileupInfo)
