void MPPCsaturation(){
  gStyle->SetOptStat("");
  TH1F pxExp("pxExp", "Expected number of pixels vs number of photons incident", 5000, 0, 5000);
  double N = 452; // area of fiber end * pixel density
  double qe = 0.2; // quantum efficiency of MPPC
  double nocc = 0; // number occupied
  for (int i=1; i<=10000; i++) {
    nocc = nocc + qe*(N-nocc)/N;
    pxExp.SetBinContent(i,nocc);
  }
  pxExp.Draw();
  pxExp.SetMaximum(500);
  pxExp.GetXaxis()->SetTitle("Number of photons incident on MPPC");
  pxExp.GetYaxis()->SetTitle("Expected number of pixels fired by MPPC");

  TMathText rcnRel;
  rcnRel.DrawMathText(500, 350, "\\langle N_{ocp} \\rangle _{i+1} = \\epsilon {N_{pix} - \\langle N_{ocp}\\rangle _{i} \\over N_{pix}}");

  gStyle->SetOptFit();
  TF1 fit1("fit1", "[0]*(1-(1-[1]/[0])**(x))", 0, 5000);
  fit1.SetParameter(0,452);
  fit1.SetParameter(1,0.2);
  pxExp.Fit("fit1");

  TMathText func;
  func.DrawMathText(1000,100,"\\langle N_{ocp}(n_{\\gamma}) \\rangle = N_{pix} \\left[ 1- \\left( 1- {\\epsilon \\over N_{pix}} \\right)^{n_{\\gamma}}  \\right]");
}
