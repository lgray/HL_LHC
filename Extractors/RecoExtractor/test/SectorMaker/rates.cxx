// Class for the rates determination
// For more info, look at the header file

#include "rates.h"

// Main constructor

rates::rates(std::string filename, std::string outfile)
{
  rates::initTuple(filename,outfile);
  rates::initVars();
  rates::get_rates();
}


//////////////////////////////////////////////
//
// 
//////////////////////////////////////////////

void rates::get_rates()
{
  // Initialize some params
 
  int B_id,E_id; // The detector module IDs (defined in the header)
  int Bl_id,El_id; 

  int disk;
  int layer,ladder,module,chip;

  float IP,PT;

  bool is_prim = false;
  bool is_prim2= false;
  bool is_fake = false;

  int st,idx,seg,nseg;
  float phi,eta,r;
  double eta_seg,phi_seg;

  int n_ss_half1;
  int n_ss_half2;
  int n_innef_ss;
  int n_cbc_innef_ss;

  int n_entries = L1TT->GetEntries();

  double fact = 1./static_cast<float>(n_entries);

  // Then loop over events

  evt_maxPSb = -1;
  evt_maxSSb = -1;
  evt_nsPSb = -1;
  evt_nsSSb = -1;

  n_max_PSb = 0;
  n_max_SSb = 0;

  for (int j=0;j<n_entries;++j)
  {
    L1TT->GetEntry(j); 
   
    if (j%8==0)
    {
      for (int i=0;i<58000;++i) // Barrel
      {   
	n_conc_half1[i]     = 0;
	n_conc_half2[i]     = 0;
      }
    }


    for (int i=0;i<6;++i)
    {
      m_bar_clus[i] = 0;
      m_bar_stub[i] = 0;
    }

    if (j%100==0) cout << j << endl;

    if (m_clus == 0) continue; // No clusters, don't go further

    for (int i=0;i<m_clus;++i)
    {
      disk  = 0;
      layer = m_clus_layer[i]; 
      ladder= m_clus_ladder[i]; 
      module= m_clus_module[i]; 

      if (layer>10 && layer<=17) disk=(layer-10)%8;
      if (layer>17 && layer<=24) disk=(layer-17)%8+7;

      B_id = (layer-5)*10000 + (ladder-1)*100 + (module-1)/2;
      E_id = (disk-1)*10000 + (ladder-1)*100 + (module-1)/2;

      Bl_id = (layer-5)*100 + (module-1)/2;
      El_id = (disk-1)*100 + (ladder-1);

      if (disk==0) // Barrel
      {
	++m_bar_clus[layer-5];
	m_b_crate[B_id]      += fact;
	m_b_bylc_rate[Bl_id] += fact;
      }
      else 
      {
	m_e_crate[E_id]      += fact;
	m_e_bylc_rate[El_id] += fact;
      }
    }

    if (m_stub == 0) continue; // No stubs, don't go further

    for (int i=0;i<58000;++i)
    { 
      tempo_ps_b[i] = 0;   
      tempo_ss_b[i] = 0; 
      
      for (int k=0;k<16;++k) tempo_c_ps_b[k][i]=0;
      for (int k=0;k<16;++k) tempo_c_ss_b[k][i]=0;
    }

    for (int i=0;i<m_stub;++i)
    {  
      // First of all we compute the ID of the stub's module

      disk  = 0;
      layer = m_stub_layer[i]; 
      ladder= m_stub_ladder[i]; 
      module= m_stub_module[i]; 
      seg   = m_stub_seg[i]; 
      chip  = m_stub_chip[i]; 
      nseg  = m_clus_nseg[m_stub_clust1[i]];

      if (seg/(nseg/2)==1) chip += 8;

      if (layer>10 && layer<=17) disk=(layer-10)%8;
      if (layer>17 && layer<=24) disk=(layer-17)%8+7;
         
      B_id = (layer-5)*10000 + ladder*100 + module;
      E_id = (disk-1)*10000 + ladder*100 + module;

      Bl_id = (layer-5)*100 + module;
      El_id = (disk-1)*100 + ladder;

      if (disk==0)
      {
	if (nseg>2)
	{ 
	  ++tempo_ps_b[B_id];
	  ++tempo_c_ps_b[chip][B_id];  
	}
	else
	{
	  ++tempo_ss_b[B_id];
	  ++tempo_c_ss_b[chip][B_id];  
	}
      }
      // Then we look if the stub is fake/secondary/primary 

      is_prim=false;
      is_prim2=false;
      is_fake=false;

      IP = sqrt(m_stub_X0[i]*m_stub_X0[i]+m_stub_Y0[i]*m_stub_Y0[i]);
      PT = sqrt(m_stub_pxGEN[i]*m_stub_pxGEN[i]+m_stub_pyGEN[i]*m_stub_pyGEN[i]);

      if (m_stub_tp[i]>=0 && IP<0.2)         is_prim=true; // OK, perfectible
      if (m_stub_tp[i]<0)                    is_fake=true;
      if (m_stub_tp[i]>=0 && IP<0.2 && PT>2) is_prim2=true; // OK, perfectible


      // Get some stub info (to get position on the module)
      idx = m_stub_clust1[i];
      st  = m_stub_strip[i];
      
      phi = atan2(m_clus_y[idx],m_clus_x[idx]);
      r   = sqrt(m_clus_y[idx]*m_clus_y[idx]+m_clus_x[idx]*m_clus_x[idx]);
      eta = -log(tan(atan2(r,m_clus_z[idx])/2.));

      if (disk==0) // Barrel
      {
	++m_bar_stub[layer-5];
	m_b_rate[chip][B_id]  += fact;
	m_b_nseg[B_id]   = m_clus_nseg[idx];
	m_b_nstrip[B_id] = m_clus_nrows[idx];
	m_b_byls_rate[Bl_id] += fact;

	if (is_fake)              m_b_rate_f[B_id] += fact; 
	if (!is_fake && !is_prim) m_b_rate_s[B_id] += fact; 
	if (is_prim)              m_b_rate_p[B_id] += fact;
	if (is_prim2)             m_b_rate_pp[B_id] += fact;

	if (st>m_b_stmax[B_id])
	{
	  m_b_phimin[B_id]=phi; 
	  m_b_stmax[B_id]=st; 
	}

	if (st<m_b_stmin[B_id])
	{
	  m_b_phimax[B_id]=phi; 
	  m_b_stmin[B_id]=st; 
	}

	if (seg>m_b_segmax[B_id]) 
        {	  
	  m_b_etamin[B_id]=eta; 
	  m_b_segmax[B_id]=seg; 	  
	}

	if (seg<m_b_segmin[B_id])
        {
	  m_b_etamax[B_id]=eta; 
	  m_b_segmin[B_id]=seg; 	  
	}	
      }
      else 
      {
	if (disk<=7)
	{
	  // Endcap +z: phi grows with strip, eta grows with seg

	  m_e_rate[chip][E_id] += fact;
	  m_e_nseg[E_id]   = m_clus_nseg[idx];
	  m_e_nstrip[E_id] = m_clus_nrows[idx];
	  m_e_bylc_rate[El_id] += fact;

	  if (is_fake)              m_e_rate_f[E_id] += fact; 
	  if (!is_fake && !is_prim) m_e_rate_s[E_id] += fact; 
	  if (is_prim)              m_e_rate_p[E_id] += fact;
	  if (is_prim2)             m_e_rate_pp[E_id] += fact;
	  
	  if (st>m_e_stmax[E_id])
	  {
	    m_e_phimax[E_id]=phi; 
	    m_e_stmax[E_id]=st; 
	  }

	  if (st<m_e_stmin[E_id])
	  {
	    m_e_phimin[E_id]=phi; 
	    m_e_stmin[E_id]=st; 
	  }

	  if (seg>m_e_segmax[E_id]) 
	  {	  
	    m_e_etamax[E_id]=eta; 
	    m_e_segmax[E_id]=seg; 	  
	  }

	  if (seg<m_e_segmin[E_id])
	  {
	    m_e_etamin[E_id]=eta; 
	    m_e_segmin[E_id]=seg; 	  
	  }	
	}
	
	if (disk>7)
	{
	  // Endcap +z: phi grows with strip, eta grows with seg

	  m_e_rate[chip][E_id] += fact;
	  m_e_nseg[E_id]   = m_clus_nseg[idx];
	  m_e_nstrip[E_id] = m_clus_nrows[idx];
	  m_e_byls_rate[El_id] += fact;

	  if (is_fake)              m_e_rate_f[E_id]  += fact; 
	  if (!is_fake && !is_prim) m_e_rate_s[E_id]  += fact; 
	  if (is_prim)              m_e_rate_p[E_id]  += fact;
	  if (is_prim2)             m_e_rate_pp[E_id] += fact;
	  
	  if (st>m_e_stmax[E_id])
	  {
	    m_e_phimin[E_id]=phi; 
	    m_e_stmax[E_id]=st; 
	  }

	  if (st<m_e_stmin[E_id])
	  {
	    m_e_phimax[E_id]=phi; 
	    m_e_stmin[E_id]=st; 
	  }

	  if (seg>m_e_segmax[E_id]) 
	  {	  
	    m_e_etamin[E_id]=eta; 
	    m_e_segmax[E_id]=seg; 	  
	  }

	  if (seg<m_e_segmin[E_id])
	  {
	    m_e_etamax[E_id]=eta; 
	    m_e_segmin[E_id]=seg; 	  
	  }	
	}
      }
    }

    n_cbc_innef_ss = 0;
    n_innef_ss     = 0;

    m_rate= 0; 
    m_disk= -1;

    for (int i=0;i<58000;++i) // Barrel
    {   
      n_ss_half1     = 0;
      n_ss_half2     = 0;

      for (int k=0;k<16;++k) 
      {
	if (tempo_c_ss_b[k][i]>3) ++n_cbc_innef_ss; 
	//	if (tempo_c_ps_b[k]>3) ++n_cbc_innef_ps; 
      }

      for (int k=0;k<8;++k) 
      {
	(tempo_c_ss_b[k][i]<=3)
	  ? n_ss_half1+=tempo_c_ss_b[k][i]
	  : n_ss_half1+=3;

	(tempo_c_ss_b[k+8][i]<=3)
	  ? n_ss_half2+=tempo_c_ss_b[k+8][i]
	  : n_ss_half2+=3;
      }

      n_conc_half1[i]    += n_ss_half1;
      n_conc_half2[i]    += n_ss_half2;

      if (n_ss_half1>22)
      {
	++n_innef_ss;
      }

      if (n_ss_half2>22) ++n_innef_ss;

      if (j%8==7)
      {
	m_disk= -2;

	if (n_conc_half1[i]>22)
	{
	  ++m_rate;
	}

	if (n_conc_half2[i]>22) ++m_rate;
      }

      if (tempo_ps_b[i]>m_b_max[i])
      {
	m_b_max[i]=tempo_ps_b[i];
	for (int k=0;k<16;++k) m_b_c_max[k][i] = tempo_c_ps_b[k][i]; 
      }

      if (tempo_ss_b[i]>m_b_max[i])
      {
	m_b_max[i]=tempo_ss_b[i];
	for (int k=0;k<16;++k) m_b_c_max[k][i] = tempo_c_ss_b[k][i]; 
      }
    }



    m_lay = 0;
    m_lad = 0; 
    m_mod = 0; 
    m_sen = 0;  
    m_chp = 0; 

    m_ss  = n_innef_ss; 
    m_cbc_ss = n_cbc_innef_ss; 
    m_dbgtree->Fill(); 


  } // End of loop over events

  // The main tree is filled up at this point
  // In the following we just fill up some infos


  // Here we fill some debug information


  for (int i=0;i<58000;++i) // Barrel
  {
    if (m_b_rate_f[i]+m_b_rate_s[i]+m_b_rate_p[i]==0.) continue;

    if (m_b_segmax[i]-m_b_segmin[i]!=0)
    {
      eta_seg = (m_b_etamax[i]-m_b_etamin[i])/(m_b_segmax[i]-m_b_segmin[i]);
      m_b_etamin[i] = m_b_etamin[i] - (m_b_segmin[i]+0.5)*eta_seg;
      m_b_etamax[i] = m_b_etamin[i] + m_b_nseg[i]*eta_seg;
    }

    if (m_b_stmax[i]-m_b_stmin[i]!=0)
    {
      if (m_b_phimax[i]<m_b_phimin[i]) m_b_phimax[i]+=8*atan(1.);

      phi_seg = (m_b_phimax[i]-m_b_phimin[i])/(m_b_stmax[i]-m_b_stmin[i]);

      m_b_phimin[i] = m_b_phimin[i] - (m_b_stmin[i]+0.5)*phi_seg;
      m_b_phimax[i] = m_b_phimin[i] + m_b_nstrip[i]*phi_seg;

      if (m_b_phimin[i]<4*atan(1.) && m_b_phimax[i]>4*atan(1.)) m_b_phimax[i]-=8*atan(1.);  
    }

    for (int j=0;j<16;++j) 
    {
      m_disk= 0;
      m_lay = static_cast<int>(i/10000);
      m_lad = static_cast<int>((i-10000*m_lay)/100);
      m_mod = static_cast<int>((i-10000*m_lay-100*m_lad));
      m_sen = j/8+1;
      m_chp = j%8+1;
      m_rate= m_b_rate[j][i];
      m_ss = 0; 
      m_cbc_ss = 0;
      m_dbgtree->Fill(); 
    }
  }

  for (int i=0;i<140000;++i) // Endcap
  {
    if (m_e_rate_f[i]+m_e_rate_s[i]+m_e_rate_p[i]==0.) continue;

    if (m_e_segmax[i]-m_e_segmin[i]!=0)
    {
      eta_seg = (m_e_etamax[i]-m_e_etamin[i])/(m_e_segmax[i]-m_e_segmin[i]);
      m_e_etamin[i] = m_e_etamin[i] - (m_e_segmin[i]+0.5)*eta_seg;
      m_e_etamax[i] = m_e_etamin[i] + m_e_nseg[i]*eta_seg;
    }


    if (m_e_stmax[i]-m_e_stmin[i]!=0)
    {
      if (m_e_phimax[i]<m_e_phimin[i]) m_e_phimax[i]+=8*atan(1.);

      phi_seg = (m_e_phimax[i]-m_e_phimin[i])/(m_e_stmax[i]-m_e_stmin[i]);

      m_e_phimin[i] = m_e_phimin[i] - (m_e_stmin[i]+0.5)*phi_seg;
      m_e_phimax[i] = m_e_phimin[i] + m_e_nstrip[i]*phi_seg;

      if (m_e_phimin[i]<4*atan(1.) && m_e_phimax[i]>4*atan(1.)) m_e_phimax[i]-=8*atan(1.);  
    }

    for (int j=0;j<16;++j) 
    {
      m_disk= 1;
      m_lay = static_cast<int>(i/10000);
      m_lad = static_cast<int>((i-10000*m_lay)/100);
      m_mod = static_cast<int>((i-10000*m_lay-100*m_lad));
      m_sen = j/8+1;
      m_chp = j%8+1;
      m_rate= m_e_rate[j][i];
      m_ss = 0; 
      m_cbc_ss = 0;
      m_dbgtree->Fill(); 
    }
  }

  // End of dbg loop, fill up root trees

  m_ratetree->Fill();  
  m_outfile->Write();
  delete L1TT;
  delete m_outfile;
}


/////////////////////////////////////////////////////////////
//
// Basic methods, initializations,...
//
/////////////////////////////////////////////////////////////


void rates::initVars()
{
  for (int i=0;i<58000;++i)
  {
    for (int j=0;j<16;++j) m_b_rate[j][i]   = 0.;
    for (int j=0;j<16;++j) m_b_c_max[j][i]   = 0;
    m_b_max[i]    = 0;
    m_b_rate_p[i] = 0.;
    m_b_rate_pp[i] = 0.;
    m_b_rate_s[i] = 0.;
    m_b_rate_f[i] = 0.;
    m_b_etamin[i] = 1000.;
    m_b_etamax[i] = -1000.;
    m_b_phimin[i] = 1000.;
    m_b_phimax[i] = -1000.;
    m_b_stmin[i]  = 2000.;
    m_b_stmax[i]  = -2000.;
    m_b_segmin[i] = 2000.;
    m_b_segmax[i] = -2000.;
    m_b_nseg[i]   = 0.;
    m_b_nstrip[i] = 0.;
    m_b_crate[i]  = 0.;
    m_b_drate[i]  = 0.;
  }

  for (int i=0;i<600;++i)
  {
    m_b_bylc_rate[i] = 0.;
    m_b_byls_rate[i] = 0.;
  }

  for (int i=0;i<1500;++i)
  {
    m_e_bylc_rate[i] = 0.;
    m_e_byls_rate[i] = 0.;
  }

  for (int i=0;i<142000;++i)
  {
    for (int j=0;j<16;++j) m_e_rate[j][i]   = 0.;
    m_e_rate_p[i] = 0.;
    m_e_rate_pp[i] = 0.;
    m_e_rate_s[i] = 0.;
    m_e_rate_f[i] = 0.;
    m_e_etamin[i] = 1000.;
    m_e_etamax[i] = -1000.;
    m_e_phimin[i] = 1000.;
    m_e_phimax[i] = -1000.;
    m_e_stmin[i]  = 2000.;
    m_e_stmax[i]  = -2000.;
    m_e_segmin[i] = 2000.;
    m_e_segmax[i] = -2000.;
    m_e_nseg[i]   = 0.;
    m_e_nstrip[i] = 0.;
    m_e_crate[i]  = 0.;
    m_e_drate[i]  = 0.;
  }
}



void rates::initTuple(std::string in,std::string out)
{
  L1TT   = new TChain("L1TrackTrigger"); 

  // Input data file

  std::size_t found = in.find(".root");

  // Case 1, it's a root file
  if (found!=std::string::npos)
  {
    L1TT->Add(in.c_str());
  }
  else // This is a list provided into a text file
  {
    std::string STRING;
    std::ifstream in2(in.c_str());
    if (!in2)
    {
      std::cout << "Please provide a valid data filename list" << std::endl;
      return;
    }
  
    while (!in2.eof())
    {
      getline(in2,STRING);

      found = STRING.find(".root");
      if (found!=std::string::npos) L1TT->Add(STRING.c_str());
    }

    in2.close();
  }

  
  pm_clus_x=&m_clus_x;
  pm_clus_y=&m_clus_y;
  pm_clus_z=&m_clus_z;
  pm_clus_layer=&m_clus_layer;
  pm_clus_ladder=&m_clus_ladder;
  pm_clus_module=&m_clus_module;
  pm_clus_nrows=&m_clus_nrows;
  pm_clus_nseg=&m_clus_nseg;

  pm_stub_layer=&m_stub_layer;
  pm_stub_ladder=&m_stub_ladder;
  pm_stub_module=&m_stub_module;
  pm_stub_tp=&m_stub_tp;
  pm_stub_pt=&m_stub_pt;
  pm_stub_pxGEN=&m_stub_pxGEN;
  pm_stub_pyGEN=&m_stub_pyGEN;
  pm_stub_etaGEN=&m_stub_etaGEN;
  pm_stub_X0=&m_stub_X0;
  pm_stub_Y0=&m_stub_Y0;
  pm_stub_x=&m_stub_x;
  pm_stub_y=&m_stub_y;
  pm_stub_z=&m_stub_z;
  pm_stub_strip=&m_stub_strip;
  pm_stub_seg=&m_stub_seg;
  pm_stub_chip=&m_stub_chip;
  pm_stub_pdgID=&m_stub_pdgID;
  pm_stub_clust1=&m_stub_clust1;


  L1TT->SetBranchAddress("STUB_n",         &m_stub);
  L1TT->SetBranchAddress("STUB_layer",     &pm_stub_layer);
  L1TT->SetBranchAddress("STUB_ladder",    &pm_stub_ladder);
  L1TT->SetBranchAddress("STUB_module",    &pm_stub_module);
  L1TT->SetBranchAddress("STUB_pxGEN",     &pm_stub_pxGEN);
  L1TT->SetBranchAddress("STUB_pyGEN",     &pm_stub_pyGEN);
  L1TT->SetBranchAddress("STUB_etaGEN",    &pm_stub_etaGEN);
  L1TT->SetBranchAddress("STUB_tp",        &pm_stub_tp);
  L1TT->SetBranchAddress("STUB_pt",        &pm_stub_pt);
  L1TT->SetBranchAddress("STUB_X0",        &pm_stub_X0);
  L1TT->SetBranchAddress("STUB_Y0",        &pm_stub_Y0);
  L1TT->SetBranchAddress("STUB_x",         &pm_stub_x);
  L1TT->SetBranchAddress("STUB_y",         &pm_stub_y);
  L1TT->SetBranchAddress("STUB_z",         &pm_stub_z);
  L1TT->SetBranchAddress("STUB_strip",     &pm_stub_strip);
  L1TT->SetBranchAddress("STUB_seg",       &pm_stub_seg);
  L1TT->SetBranchAddress("STUB_chip",      &pm_stub_chip);
  L1TT->SetBranchAddress("STUB_pdgID",     &pm_stub_pdgID);
  L1TT->SetBranchAddress("STUB_clust1",    &pm_stub_clust1);

  L1TT->SetBranchAddress("CLUS_n",         &m_clus);
  L1TT->SetBranchAddress("CLUS_layer",     &pm_clus_layer);
  L1TT->SetBranchAddress("CLUS_ladder",    &pm_clus_ladder);
  L1TT->SetBranchAddress("CLUS_module",    &pm_clus_module);
  L1TT->SetBranchAddress("CLUS_x",         &pm_clus_x);
  L1TT->SetBranchAddress("CLUS_y",         &pm_clus_y);
  L1TT->SetBranchAddress("CLUS_z",         &pm_clus_z);
  L1TT->SetBranchAddress("CLUS_nrows",     &pm_clus_nrows);
  L1TT->SetBranchAddress("CLUS_PS",        &pm_clus_nseg);

  m_outfile  = new TFile(out.c_str(),"recreate");
  m_ratetree = new TTree("L1Rates","L1Rates info");
  m_dbgtree  = new TTree("Details","Debug");


  m_ratetree->Branch("STUB_b_rates",         &m_b_rate,      "STUB_b_rates[16][58000]/F");
  m_ratetree->Branch("STUB_b_c_max",         &m_b_c_max,     "STUB_b_c_max[16][58000]/I");
  m_ratetree->Branch("STUB_b_max",           &m_b_max,       "STUB_b_max[58000]/I");
  m_ratetree->Branch("STUB_b_rates_prim2",   &m_b_rate_pp,   "STUB_b_rates_prim2[58000]/F"); 
  m_ratetree->Branch("STUB_b_rates_prim",    &m_b_rate_p,    "STUB_b_rates_prim[58000]/F"); 
  m_ratetree->Branch("STUB_b_rates_sec",     &m_b_rate_s,    "STUB_b_rates_sec[58000]/F"); 
  m_ratetree->Branch("STUB_b_rates_f",       &m_b_rate_f,    "STUB_b_rates_fake[58000]/F"); 
  m_ratetree->Branch("STUB_b_phi_b",         &m_b_phimin,    "STUB_b_phi_b[58000]/F"); 
  m_ratetree->Branch("STUB_b_phi_t",         &m_b_phimax,    "STUB_b_phi_t[58000]/F"); 
  m_ratetree->Branch("STUB_b_eta_b",         &m_b_etamin,    "STUB_b_eta_b[58000]/F"); 
  m_ratetree->Branch("STUB_b_eta_t",         &m_b_etamax,    "STUB_b_eta_t[58000]/F"); 
  m_ratetree->Branch("CLUS_b_rates",         &m_b_crate,     "CLUS_b_rates[58000]/F");
  m_ratetree->Branch("DIGI_b_rates",         &m_b_drate,     "DIGI_b_rates[58000]/F");
  m_ratetree->Branch("STUB_b_l_rates",       &m_b_byls_rate, "STUB_b_l_rates[600]/F");
  m_ratetree->Branch("CLUS_b_l_rates",       &m_b_bylc_rate, "CLUS_b_l_rates[600]/F");

  m_ratetree->Branch("STUB_e_rates",         &m_e_rate,      "STUB_e_rates[16][142000]/F");
  m_ratetree->Branch("STUB_e_rates_prim2",    &m_e_rate_pp,    "STUB_e_rates_prim2[142000]/F"); 
  m_ratetree->Branch("STUB_e_rates_prim",    &m_e_rate_p,    "STUB_e_rates_prim[142000]/F"); 
  m_ratetree->Branch("STUB_e_rates_sec",     &m_e_rate_s,    "STUB_e_rates_sec[142000]/F"); 
  m_ratetree->Branch("STUB_e_rates_f",       &m_e_rate_f,    "STUB_e_rates_fake[142000]/F"); 
  m_ratetree->Branch("STUB_e_phi_b",         &m_e_phimin,    "STUB_e_phi_b[142000]/F"); 
  m_ratetree->Branch("STUB_e_phi_t",         &m_e_phimax,    "STUB_e_phi_t[142000]/F"); 
  m_ratetree->Branch("STUB_e_eta_b",         &m_e_etamin,    "STUB_e_eta_b[142000]/F"); 
  m_ratetree->Branch("STUB_e_eta_t",         &m_e_etamax,    "STUB_e_eta_t[142000]/F"); 
  m_ratetree->Branch("CLUS_e_rates",         &m_e_crate,     "CLUS_e_rates[142000]/F");
  m_ratetree->Branch("DIGI_e_rates",         &m_e_drate,     "DIGI_e_rates[142000]/F");
  m_ratetree->Branch("STUB_e_l_rates",       &m_e_byls_rate, "STUB_e_l_rates[1500]/F");
  m_ratetree->Branch("CLUS_e_l_rates",       &m_e_bylc_rate, "CLUS_e_l_rates[1500]/F");

  m_dbgtree->Branch("disk",        &m_disk,   "disk/I"); 
  m_dbgtree->Branch("lay",         &m_lay,    "lay/I"); 
  m_dbgtree->Branch("lad",         &m_lad,    "lad/I"); 
  m_dbgtree->Branch("mod",         &m_mod,    "mod/I"); 
  m_dbgtree->Branch("sen",         &m_sen,    "sen/I"); 
  m_dbgtree->Branch("chip",        &m_chp,    "chip/I"); 
  m_dbgtree->Branch("rate",        &m_rate,   "rate/F"); 
  m_dbgtree->Branch("cbc_o_3",     &m_cbc_ss, "cbc_o_3/I"); 
  m_dbgtree->Branch("conc_o_12",   &m_ss,     "conc_o_12/I"); 
  m_dbgtree->Branch("rate",        &m_rate,   "rate/F");
  m_dbgtree->Branch("bar_c_mult",  &m_bar_clus,"m_bar_clus[6]/I"); 
  m_dbgtree->Branch("bar_s_mult",  &m_bar_stub,"m_bar_stub[6]/I"); 
}
