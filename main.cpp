/*
	ab plugin
	main.c
	This file brings everything together into a DLL
	Kevin Dixon
	12/16/06
*/
#include <windows.h>
#include <stdio.h>
#include "dsp.h"
#include "resource.h"

#ifdef __cplusplus
# define DLLEXPORT extern "C" __declspec( dllexport )
#else
# define DLLEXPORT __declspec( dllexport )
#endif

//global variables
extern winampDSPModule ab;
//define operation modes:
#define OP_BYPASS 0
#define OP_PLUS 1
#define OP_MINUS 2
volatile int g_op;
//define output modes
#define OUT_A 1
#define OUT_B 2
#define OUT_BOTH 3
volatile int g_out;

HWND hConfigDialog = 0;

//function headers
winampDSPModule * getModule(int);
void config(struct winampDSPModule *);
BOOL CALLBACK ConfigDialogProc(HWND, UINT, WPARAM, LPARAM);
void config_setup(HWND);
void config_bypassChecked(HWND);
void config_opChecked(HWND);
void config_outChecked(HWND);
int init_ab( struct winampDSPModule *);
int apply_ab( struct winampDSPModule *, short int *, int, int, int, int);
void quit_ab( struct winampDSPModule *);

/*
	Define our module
*/
winampDSPModule ab = {
	"A+/-B Analysis Tool (2006 Kevin Dixon)",
	NULL,			// hwndParent
	NULL,			// hDllInstance
	config,
	init_ab,
	apply_ab,
	quit_ab
};

/*
	Create winampDSPHeader
	Tells version, name and how to get at the modules to winamp
*/
winampDSPHeader header =
{
	DSP_HDRVER,
	"A+/-B Analysis Tool (2006 Kevin Dixon)",
	getModule
};


/*
	Returns the library header to Winamp
*/
DLLEXPORT winampDSPHeader * winampDSPGetHeader2() {
	return &header;
}

/*
	Returns the module to use, as requested by winamp
*/
winampDSPModule * getModule(int which) {
	if(which == 0)
		return &ab;
	return NULL;
}

//---- GUI ---------------------------------------------------------------------
/*
	Show configuration window when winamp demands it
*/
void config(struct winampDSPModule * this_mod) {
	hConfigDialog = CreateDialog(this_mod->hDllInstance, MAKEINTRESOURCE(dCONFIG), this_mod->hwndParent, ConfigDialogProc);
}

/*
	Callback process for the configuration dialog box
*/
BOOL CALLBACK ConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		//respond to initialize dialog
		case WM_INITDIALOG:
			config_setup(hwnd);
			return FALSE;
		//respond to actions on controls
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_BYPASS:
				case IDC_PLUS:
				case IDC_MINUS:
					config_opChecked(hwnd);
					break;
				case IDC_BOTH:
				case IDC_A:
				case IDC_B:
					config_outChecked(hwnd);
					break;
				case IDOK:
				case IDCANCEL:
					EndDialog(hwnd, TRUE);
					break;
			}
			return TRUE;

		case WM_CLOSE:
			DestroyWindow(hwnd);
			return TRUE;
		case WM_DESTROY:
			EndDialog(hwnd, TRUE);
			return TRUE;	
	}
	return FALSE;
}

/*
	Sets up the options window when it appears
*/
void config_setup(HWND hwnd) {
	//setup operation buttons
	int check = IDC_BYPASS;
	if(g_op == OP_PLUS)
		check = IDC_PLUS;
	else if(g_op == OP_MINUS)
		check = IDC_MINUS;
	else
		g_op = OP_BYPASS;
		
	CheckRadioButton(hwnd, IDC_BYPASS, IDC_MINUS, check);
	
	//setup output channel
	check = IDC_BOTH;
	if(g_out == OUT_A)
		check = IDC_A;
	else if(g_out == OUT_B)
		check = IDC_B;
	else
		g_out = OUT_BOTH;
	
	CheckRadioButton(hwnd, IDC_BOTH, IDC_B, check);
}

/*
	Changes the operation performed when an operation is changed
*/
void config_opChecked(HWND hwnd) {
	if(IsDlgButtonChecked(hwnd, IDC_BYPASS))
		g_op = OP_BYPASS;
	else if(IsDlgButtonChecked(hwnd, IDC_PLUS))
		g_op = OP_PLUS;
	else if(IsDlgButtonChecked(hwnd, IDC_MINUS))
		g_op = OP_MINUS;
}

/*
	Changes the selected output channel when a button is selected
*/
void config_outChecked(HWND hwnd) {
	if(IsDlgButtonChecked(hwnd, IDC_BOTH))
		g_out = OUT_BOTH;
	else if(IsDlgButtonChecked(hwnd, IDC_A))
		g_out = OUT_A;
	else if(IsDlgButtonChecked(hwnd, IDC_B))
		g_out = OUT_B;
}

//---- DSP ---------------------------------------------------------------------
/*
	Perform initialization
	default to bypassed, both speakers output
*/
int init_ab(struct winampDSPModule * this_mod) {
	g_op = OP_BYPASS;
	g_out = OUT_A | OUT_B;
	
	return 0;
}

/*
	apply the effect
	this_mod		pointer to the module
	samples		the samples
	numsamples	number of samples
	bps			bits per sample
	nch			number of channels
	srate		sample rate
*/
int apply_ab( struct winampDSPModule * this_mod, short int * samples, int numsamples, int bps, int nch, int srate )
{
	//if not in stereo mode, or if bypassed, do nothing
	if((nch != 2) || (g_op == OP_BYPASS))
		return numsamples;
		
	//calculate block length
	const int blockLength = numsamples * 2;
	//perform operations on pairs of samples
	for(int i = 0; i < blockLength; i += 2) {
		int sum = 0;
		//add samples
		if(g_op == OP_PLUS)
			sum = samples[i] + samples[i + 1];
		//invert B sample and add
		else if(g_op == OP_MINUS)
			sum = samples[i] - samples[i + 1];
			
		//place samples into the channels we want to hear from
		if (sum < -32768) sum = -32768;
		if (sum > 32767) sum = 32767;
		samples[i] = (g_out & OUT_A)?sum:0;
		samples[i+1] = (g_out & OUT_B)?sum:0;
	}

	return numsamples;
}

/*
	Cleanup on quit
*/
void quit_ab(struct winampDSPModule * this_mod) {
	DestroyWindow(hConfigDialog);
}

