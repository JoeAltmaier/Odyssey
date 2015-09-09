/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: diagmsg.c
// 
// Description:
// This file contains all diagnostics message strings.
// 
// 
// Update Log 
// 
// 07/27/99 Bob Weast: Create file
// 
/*************************************************************************/



/* ======================= MENU ITEMS ==============================*/

char *diag_title = 
  "\033[02;15HConvergeNet Technologies-%s Diagnostics Menu. (v0.02)";

char *allTest1 = "\033[05;25H1) Main Memory Test";
char *allTest2 = "\033[06;25H2) Image Flash Test";

char *hbcTest3 = "\033[07;25H3) Realtime Clock Test";
char *hbcTest4 = "\033[08;25H4) NAND Flash Test";
char *hbcTest5 = "\033[09;25H5) EMAC_0 Register Test";
char *hbcTest6 = "\033[10;25H6) EMAC_1 Register Test";
char *hbcTest7 = "\033[11;25H7) Eth0 Internal LB Test";
char *hbcTest8 = "\033[12;25H8) Eth1 Internal LB Test";
char *hbcTestT = "\033[13;25HT) Toolbox";

char *nacTest3 = "\033[07;25H3) FC_MAC_0 Register Test";
char *nacTest4 = "\033[08;25H4) FC_MAC_1 Register Test";
char *nacTest5 = "\033[09;25H5) FC_MAC_2 Register Test";
char *nacTestT = "\033[10;25HT) Toolbox";

char *ssdTest3 = "\033[07;25H3) NAND Flash Test (Bank 0)";
char *ssdTest4 = "\033[08;25H4) NAND Flash Test (Bank 1)";
char *ssdTest5 = "\033[09;25H5) NAND Flash Test (Bank 2)";
char *ssdTest6 = "\033[10;25H6) NAND Flash Test (Bank 3)";
char *ssdTestT = "\033[11;25HT) Toolbox";

char *allCmdA  = "\033[07;09H(A)ll";
char *allCmdB  = "\033[08;09H(B)urnin";
char *allCmdO  = "\033[09;09H(O)ptions";



/* ================== MISCELLANEOUS STRINGS ========================*/

char *names[] = {"Unk", "HBC", "NAC", "SSD", "NIC", "RAC"};
char *aoknl = "OK\n";
char *keepGoing = "Continue?";
char *pressAnyKey = "Press any key to continue...";
char *pressReturn = "\nPress RETURN to continue...";
char *sysTestOK = "\nFunctional Test passed\n";
char *sysTestBAD = "\nFunctional Test FAILED!\n";
char *burnOK = "\nBurnin Terminated Successfully, %ld passes complete.\n";
char *burnBAD = "\nBurnin FAILED!\n";
char *lessq = "More? ";
char *changeq = ", change?";
char *nl = "\n";
char *nl2 = "\n\n";
char *ttr = "Times To Run";
char *verb = "Verbose";



/* ===================== ERROR MESSAGES ============================*/

char *dmc = "Data Miscompare in ";

char *errAddrWR_byte = "Error at location %X, wrote %02x, read %02x\n";
char *errAddrWR_half = "Error at location %X, wrote %04x, read %04x\n";
char *errAddrWR_word = "Error at location %X, wrote %08x, read %08x\n";
char *errAddrWR_long = "Error at location %X,\n wrote %016Lx, read %016Lx\n";



/* ======================== VERBOSITY ==============================*/

char *verbMemTstHerald = "Dword (%016Lx) ramp test from %X to %X\n";
char *verbDoingBytes = "Testing byte accesses...";
char *verbDoingHalves = "Testing short accesses...";
char *verbDoingWords = "Testing word accesses...";
char *verbDoingLongs = "Testing long accesses...";
char *verbWriting = "Writing.....";
char *verbReadCompare = "Reading/Comparing.....";

