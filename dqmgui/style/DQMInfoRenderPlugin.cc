/*!
  \file DQMInfoRenderPlugin.cc
  \\
  \\ Code shamelessly borrowed from J. Temple's HcalRenderPlugin.cc code,
  \\ which was shamelessly borrowed from S. Dutta's SiStripRenderPlugin.cc
  \\ code, G. Della Ricca and B. Gobbo's EBRenderPlugin.cc, and other existing
  \\ subdetector plugins
  \\ preDraw and postDraw methods now check whether histogram was a TH1
  \\ or TH2, and call a private method appropriate for the histogram type
*/

#include "DQM/DQMRenderPlugin.h"
#include "utils.h"

#include "TMath.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TText.h"
#include <cassert>
#include "TROOT.h"
#include "TObjString.h"

class DQMInfoRenderPlugin : public DQMRenderPlugin
{
public:
  // These functions may be different that parent version
  virtual bool applies(const VisDQMObject &o, const VisDQMImgInfo &)
    {
      // determine whether core object is an Info object
      if ( o.name.find( "Info/EventInfo/reportSummaryMap" ) != std::string::npos
        || o.name.find( "Info/LhcInfo/") != std::string::npos
        || o.name.find( "Info/ProvInfo/Taglist") != std::string::npos)
        return true;
      return false;
    }

  virtual void preDraw (TCanvas * c, const VisDQMObject &o, const VisDQMImgInfo &, VisDQMRenderInfo &){
      c->cd();
      gPad->SetLogy(0);
      if ( o.name.find( "Info/EventInfo/reportSummaryMap" ) != std::string::npos ){
        // object is TH2 histogram
        if ( dynamic_cast<TH2F*>( o.object ) )
        {
           gPad->SetLogy(0);
           preDrawTH2F( c, o );
        }
      }
      else if ( o.name.find( "Info/LhcInfo/") != std::string::npos ){
        if ( dynamic_cast<TH1F*>( o.object ) )
        {
         gPad->SetLogy(0);
         preDrawTH1F( c, o);
        }
      }
      else if (o.name.find( "Info/ProvInfo/Taglist") != std::string::npos){
        TObjString* s;
        s=dynamic_cast<TObjString*>(o.object);
        taglist=s->String();
        s->SetString("");
           }
  }

 virtual void postDraw( TCanvas *c, const VisDQMObject &o, const VisDQMImgInfo & )
    {
      if (o.name.find( "Info/ProvInfo/Taglist") != std::string::npos){
        int max = 0;
        int length = 0;
        std::vector<std::string> tltable;
        tltable.push_back("");
        for (std::string::iterator it=taglist.begin(); it < taglist.end(); it++){
          if (*it == ';'){
            tltable.push_back("");
            length=0;
            continue;
          }
          if (*it == ':'){
            if (max < length) max = length;
            length *= -1;
          }
          tltable.back().append(1,*it);
          if (length >=0) length++;
        }
        if (tltable.back().empty()) tltable.pop_back();
        int numRows = (int) tltable.size();
        float rowHight = 1.0 / numRows;
        if (rowHight >= 0.2) rowHight = 0.08;
        TText tt;
        tt.SetTextSize(rowHight);
        tt.SetTextFont(102);
        tt.SetTextAlign(13);
        for (int i = 0; i < (int) tltable.size(); i++ ){
          int cpos = tltable[i].find(':');
          for (int j=0;j < max-cpos && cpos!=(int)std::string::npos; j++)
            tltable[i].insert(cpos+j,1,' ');
          tt.DrawText(0.01,1.0-(rowHight*i),tltable[i].c_str());
        }
        return;
      }

      return;
      c->cd();
      if ( o.name.find( "Info/EventInfo/reportSummaryMap" ) != std::string::npos )
      {
         // object is TH2 histogram
         TH2F* obj = dynamic_cast<TH2F*>( o.object );
         assert( obj );

         int topbin = obj->GetNbinsY();
         for ( int i = 1 ; i < obj->GetNbinsX(); i++ )
         {
           {
             if ( obj->GetBinContent(i,topbin-1) == 1 )
             {
               char s[25];
               sprintf (s,"Collisions10");
               TText tt1;
               tt1.SetTextSize(0.15);
               tt1.SetTextColor(13);
               tt1.DrawTextNDC(0.2,0.5,const_cast<char*>(s));
               if ( obj->GetBinContent(i,topbin-2) == 1 )
               {
                 sprintf (s,"7 TeV");
                 TText tt2;
                 tt2.SetTextSize(0.1);
                 tt2.SetTextColor(13);
                 tt2.DrawTextNDC(0.4,0.4,const_cast<char*>(s));
               }
               break;
             }
           }
         }
      }
      else if ( o.name.find( "Info/LhcInfo/beamMode") != std::string::npos )
      {
         TH1F* obj = dynamic_cast<TH1F*>( o.object );
         assert( obj );

         std::string smode[22] =
         {
          "",
          "no mode",
          "setup",
          "inj pilot",
          "inj intr",
          "inj nomn",
          "pre ramp",
          "ramp",
          "flat top",
          "squeeze",
          "adjust",
          "stable beams",
          "unstable",
          "beam dump",
          "ramp down",
          "recovery",
          "inj dump",
          "circ dump",
          "abort",
          "cycling",
          "warn beam dump",
          "no beam"
         };

         int ibin=0;
         for ( int i = obj->GetNbinsX(); i > 0; --i )
         {
           if ( obj->GetBinContent(i) != 0 )
           {
              ibin = (int)obj->GetBinContent(i);
              break;
           }
         }

         if (ibin>2)
         {
           const char* s = smode[ibin].c_str();
           TText tt;
           tt.SetTextSize(0.08);
           tt.SetTextColor(2);
           if (ibin == 11)
             tt.SetTextColor(3);
           tt.DrawTextNDC(0.3,0.5,s);
         }

      }
      else if ( o.name.find( "Info/LhcInfo/lhcFill") != std::string::npos )
      {
         TH1F* obj = dynamic_cast<TH1F*>( o.object );
         assert( obj );

         int y=0;
         for ( int i = obj->GetNbinsX(); i > 0; --i )
         {
           if ( obj->GetBinContent(i) != 0 )
           {
              y = (int)obj->GetBinContent(i);
              break;
           }
         }

         if (y!=0 && y<9999)
         {
           char s[25];
           sprintf (s,"LHC Fill: %d",y);
           TText tt;
           tt.SetTextSize(0.06);
           tt.SetTextColor(4);
           tt.DrawTextNDC(0.2,0.5,const_cast<char*>(s));
         }
      }
      else if ( o.name.find( "Info/LhcInfo/momentum") != std::string::npos )
      {
         TH1F* obj = dynamic_cast<TH1F*>( o.object );
         assert( obj );

         int y=0;
         for ( int i = obj->GetNbinsX(); i > 0; --i )
         {
           if ( obj->GetBinContent(i) != 0 )
           {
              y = (int)obj->GetBinContent(i);
              break;
           }
         }

         if (y!=0 && y<9999)
         {
           char s[25];
           sprintf (s,"Beam Energy: %d GeV",y);
           TText tt;
           tt.SetTextSize(0.06);
           tt.SetTextColor(4);
           tt.DrawTextNDC(0.2,0.5,const_cast<char*>(s));
         }
      }

    }
private:
  std::string taglist;
  void preDrawTH2F ( TCanvas *, const VisDQMObject &o )
    {
      TH2F* obj = dynamic_cast<TH2F*>( o.object );
      assert( obj );

      // Because we don't want to trigger the shifters on subdetectors that
      // are "not in" for a longer time, we want to hide them from the HV plot.
      // In practice, this need was created by ZDC and CASTOR not being in for
      // a longer period.
      // - Either we remove these subdetectors from the plot in CMSSW at the
      //   source where the plot is created.
      // - Or we just remove them when the plot is displayed in the GUI.
      // We decided to go for the second option, since it makes more sence to
      // keep all data related to the DTS bits available in the data.
      // Doing the "masking" in the renderplugin in the GUI also gives us a
      // lot more flexibility in terms of fast turning it on or of.

      // The difficulty is in the hiding of certain horizontal lines when the
      // plot is rendered. There is not really any option for that in root.
      // The only way is to actually create a new plot with different data.
      // We will first clone the plot "obj" to have the original data.
      // Then we will determine the dimensions of our updated plot.
      // And finally copy the data from the cloned plot back to our original
      // plot, but leaving out the unwanted subdetectors.

      // Clone the plot:
      TH2F *myClonedPlot = dynamic_cast<TH2F*>(obj->Clone("Cloned"));
      // And reset the original:
      obj->Reset();

      // Determine the dimensions of the new plot:
      int maxBinX = myClonedPlot->GetNbinsX();
      int maxBinY = myClonedPlot->GetNbinsY();

      // We scan from right to left to see which is the rightmost bin with data
      for ( int binX = myClonedPlot->GetNbinsX(); binX > 0; --binX )
      {
        // For this scan, we look at the top line, corresponding to "Valid"
        if ( myClonedPlot->GetBinContent(binX,maxBinY) != 0 )
        {
          // Set the range of the plot to span up to the last filled bin of
          // the top row + an extra column we will add:
          maxBinX = TMath::Max(binX + 1, 2);  // leave at least 2 bins
          break;
        }
      }

      // Now we basically copy the data from the cloned plot back into the
      // original one.
      int binYNew = 0;
      for ( int binYOrig = 1; binYOrig <= maxBinY; binYOrig++ ) {
        TString label = TString(myClonedPlot->GetYaxis()->GetBinLabel(binYOrig));
        // This is where we do the filtering: We skip the lines with the
        // following labels
        if (label != "CASTOR" && label != "ZDC") { // enable this line to filter CASTOR and ZDC away
        ////if (true) {
          binYNew++;
          for ( int binX = 1; binX < maxBinX; binX++ ) {
            // Just copy the data, with possible offset in Y
            obj->SetBinContent(binX, binYNew, myClonedPlot->GetBinContent(binX, binYOrig));
          }
          // Of course, we have to update the label as well
          obj->GetYaxis()->SetBinLabel(binYNew, label);
          // We make an extra white band to the right of the plot for better
          // visibility. (Was originally done in the code, but this belongs
          // more in the render plugin.)
          obj->SetBinContent(maxBinX, binYNew, -1.);
        }
      }

      // We set the ranges to plot, for Y this is the _new_ range:
      obj->GetXaxis()->SetRange(1,maxBinX);
      obj->GetYaxis()->SetRange(1,binYNew);

      gPad->SetGrid(1,1);
      gPad->SetLeftMargin(0.12);
      obj->SetStats( kFALSE );

      dqm::utils::reportSummaryMapPalette(obj);
      obj->SetOption("col");

      return;

    }

  void preDrawTH1F ( TCanvas *, const VisDQMObject &o )
    {
      TH1F* obj = dynamic_cast<TH1F*>( o.object );
      assert( obj );

      int nbins = obj->GetNbinsX();
      int maxRange = nbins;
      for ( int i = nbins; i > 0; --i )
      {
        if ( obj->GetBinContent(i) != 0 )
        {
           maxRange = TMath::Max(i+1,2);
           break;
        }
      }

      obj->GetXaxis()->SetRange(1,maxRange);
      obj->SetStats( kFALSE );
      obj->SetMinimum(-1.e-15);

      if ( o.name.find( "Info/LhcInfo/beamMode") != std::string::npos )
      {
        obj->SetMaximum(22.);
        obj->SetMinimum(0.);
        gPad->SetLeftMargin(0.15);
        gPad->SetGrid(1,1);
      }

      return;

    }
};

static DQMInfoRenderPlugin instance;
