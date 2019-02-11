//---------------------------------------------------------------------------

#include <vcl.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#pragma hdrstop

#include "mainwin.h"
#include "com_serial.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

#define SDEBUG  // If defined: log complete i/o to file slog.dat

#ifdef SDEBUG

static FILE *_slf;
static int _ser_dir;
static CRITICAL_SECTION _ser_critical;
#define _S_WRITE    1
#define _S_READ    -1

void _ser_log(uint8_t* pu, int len,int dir){
	if(!_slf){
		char str[128];
		sprintf(str,"slog_%u.dat",time(NULL));
		_slf=fopen(str,"wb");
		if(!_slf) return;
		InitializeCriticalSectionAndSpinCount(&_ser_critical,1);
	}
	EnterCriticalSection(&_ser_critical);
	if(dir==_S_WRITE){
		if(_ser_dir!=_S_WRITE){
			_ser_dir=_S_WRITE;
			fprintf(_slf,"WRITE:\n");
		}
	}else{
		if(_ser_dir!=_S_READ){
			_ser_dir=_S_READ;
			fprintf(_slf,"READ:\n");
		}
	}

	// fwrite(pu,1,len,_slf); // Binary
	// or HEX:
	fprintf(_slf,"[%d]", len);
	while(len--){
		fprintf(_slf," %02X",*pu++);
	}
	fprintf(_slf,"\n");

	LeaveCriticalSection(&_ser_critical);
}

#endif




//---------------Globals --------------------------------------
typedef union {
		int ival;
		unsigned int uval;
		float fval;
		unsigned char bytes[4]; // Wenn bytes[0]==FD (LE): Fehler
} MDATA;   // Datenfeld, variabel

SERIAL_PORT_INFO spi;
bool conflag=false;
uint8_t iframe[256];    // Kompletter frame ohne Rand
volatile int ifidx=-1;  // Inframe-Index
int iflen=-1;
uint8_t iesc;       // Flag Escape

//------------- C part -------------------

// -------------- Serial Reader ------------------
// Read Frame on Demand
void ext_xl_SerialReaderCallback(unsigned char *pc, unsigned int anz){
	unsigned int i,j;
	uint8_t xcs;
	uint8_t c;
	for(i=0;i<anz;i++){
		c=*pc++;

		if(ifidx==1000) continue;   // 1000: Ready!
		if(ifidx<0){
			if(c!=0x7e) continue;
			ifidx=0;
			iesc=0;
			continue;
		}
		if(c==0x7e){    // Wrong Byte or END
			if(ifidx>4 && iframe[3]==ifidx-5){
				xcs=0;
				for(j=0;j<ifidx-1;j++) xcs+=iframe[j];
				if((xcs^0xFF) == iframe[ifidx-1]){
					iflen=ifidx;    // Save len
					ifidx=1000;
					continue;
				}
			}
			iflen=-1;
			ifidx=-1;
			continue;
		}
		if(c==0x7d){
			iesc=1;
			continue;
		}
		if(iesc){
			iesc=0;
			switch(c){
			case 0x5e: c=0x7e; break;
			case 0x5d: c=0x7d; break;
			case 0x31: c=0x11; break;
			case 0x33: c=0x13; break;
			default:
				ifidx=-1;
				iflen=-1;
				continue;
			}
		}
		iframe[ifidx++]=c;
		if(ifidx>250) ifidx=-1; // ERROR
	}
}
uint8_t sframe[256];  // Send-Frame Raw
uint8_t cframe[256];

int send_frame(uint8_t cmd, uint8_t datalen){
	uint8_t *pu=cframe;
	int i,cnt;
	uint8_t xcs=0,c, resplen, status;


	sframe[0]=0;    // ADDR
	sframe[1]=cmd;
	sframe[2]=datalen;

	for(i=0;i<datalen+3;i++){
		xcs+=sframe[i];
	}
	sframe[datalen+3]=xcs^0xFF;

	*pu++=0x7E;   // Startet mit 7E
	for(i=0;i<datalen+4;i++){
		c=sframe[i];
		switch(c){
		case 0x7e: *pu++=0x7d; *pu++=0x5e; break;
		case 0x7d: *pu++=0x7d; *pu++=0x5d; break;
		case 0x11: *pu++=0x7d; *pu++=0x31; break;
		case 0x13: *pu++=0x7d; *pu++=0x33; break;
		default: *pu++=c;
		}
	}
	*pu++=0x7E; // Ende mit 7E

	datalen=pu-cframe;

	ifidx=-1;
	SerialWriteCommBlock(&spi,cframe, datalen);
#ifdef SDEBUG
	_ser_log(cframe,datalen,_S_WRITE);
#endif
	cnt=50; // 500 msec wait
	for(;;){
		if(ifidx==1000) break;  // 1000: OK
		if(!--cnt){
			Form1->Console->Lines->Add("*** No Reply ***");
			return -1;
		}
		Sleep(10);  // mse
	}
#ifdef SDEBUG
	_ser_log(iframe,iflen,_S_READ);
#endif

	status=iframe[2];
	switch(status){
	case 0: Form1->Console->Lines->Add("OK"); return 0;
	case 1: Form1->Console->Lines->Add("ERROR: Too much Data"); break;
	case 2: Form1->Console->Lines->Add("ERROR: Unknown CMD"); break;
	case 3: Form1->Console->Lines->Add("ERROR: No access right"); break;
	case 4: Form1->Console->Lines->Add("ERROR: Illegal parameter or not allowed"); break;
	case 0x28: Form1->Console->Lines->Add("ERROR: internal function/argument out of range"); break;
	case 0x43: Form1->Console->Lines->Add("ERROR: command not allowed in current state"); break;
	}
	return -1;  // ERROR
}

float get_float32(uint8_t *pu){
		MDATA hval;
		int ubexp;        // Unbias-Exponent ist zu pruefen

		hval.bytes[0]=pu[3];
		hval.bytes[1]=pu[2];
		hval.bytes[2]=pu[1];
		hval.bytes[3]=pu[0];
		if(hval.uval==0x80000000){      // Int32-Fehler?
			hval.fval=-98;
		}else{
				// 2 Spezialfaelle: NAN als Zahl abfangen und Pseudo-0 (fehlerhafte 'pseudo'-0)
				ubexp=((hval.uval>>23)&0xFF);
				if(ubexp>=254) hval.fval=-99;
				else if(ubexp==0) hval.fval=0;
		}
		return hval.fval;
}

//--------------CPP part ----------------------------
// Msg-Box
void MsgInfo(char *pform, ...){
   char str[256];
   va_list argptr;
   va_start(argptr, pform);
   vsnprintf(str, 255, pform, argptr);	// vsn: begrenzt!
   va_end(argptr);
   MessageBoxA(0,str,"Message",MB_OK);  // A fuer Ascii
}


void __fastcall TForm1::Enabler(void){
	RunBut->Enabled=!conflag;
	ComCombo->Enabled=!conflag;

	StopBut->Enabled=conflag;

	ReadIDBut->Enabled=conflag;
	SendStartBut->Enabled=conflag;
	SendStopBut->Enabled=conflag;
	SendCleanBut->Enabled=conflag;
	ReadValuesBut->Enabled=conflag;
	ResetBut->Enabled=conflag;

	ButtonReadAutoCleaning->Enabled=conflag;
	ButtonSetAutoCleaning->Enabled=conflag;
	EditSecs->Enabled=conflag;

}

//----------------CPP-FKT-----------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ExitButClick(TObject *Sender)
{
	FILE *outf;
	if(spi.com_nr){
		outf=fopen("sps30_win.inf","w");
		if(outf){
			fprintf(outf,"%d (COM)\n",spi.com_nr);
			fclose(outf);
		}
	}
	Close();
}


//---------------------------------------------------------------------------
void __fastcall TForm1::FormActivate(TObject *Sender)
{
	int cn,cx=0;
	FILE *inf;
	char coms[16]="";
	UnicodeString s;


	inf=fopen("sps30_win.inf","r");
	if(inf){
			fgets(coms,8,inf);
			fclose(inf);
	}
	cn=atoi(coms);

	ComCombo->Clear();
	int com_cnt=0;
	for(int i=0;i<256;i++){
		if(SerialTest(i)==0){
			if(i==cn) cx=com_cnt;
			s.sprintf(L"COM%d:",i);
			ComCombo->Items->Add(s);
			com_cnt++;
		}
	}
	if(!com_cnt){
			ComCombo->Items->Add("(No COM available)");
			RunBut->Enabled=false;
	}
	ComCombo->ItemIndex=cx;
	Enabler();
}

//---------------------------------------------------------------------------



void __fastcall TForm1::RunButClick(TObject *Sender)
{
	int idx;
	char val[20];
	idx=ComCombo->ItemIndex;
	sprintf(val,"%ls",ComCombo->Items->Strings[idx].c_str());
	spi.com_nr=atoi(val+3);
	int res=SerialOpen(&spi);


	if(res) {
		MsgInfo("Error %d: 'open COM%d:'",res, spi.com_nr);
		conflag=false;
	}else conflag=true;
	Enabler();
	Console->Lines->Add("Connected");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::StopButClick(TObject *Sender)
{
	SerialClose(&spi);
	conflag=false;
	Enabler();
	Console->Lines->Add("Disconnected");


}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------


void __fastcall TForm1::Button1Click(TObject *Sender)
{
		ShellExecute(NULL, L"open", L"http://www.joembedded.de", NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ReadIDButClick(TObject *Sender)
{
	int res;
	char resp[256];
	Console->Lines->Add("-> Read Sensor ID");

	sframe[3]=3;    // 3 is ID
	res=send_frame(0xD0,1);
	if(!res){
		iframe[4+32]=0; // Limit String!
		sprintf(resp,"ID: '%s'",(char*)&iframe[4]);
		Console->Lines->Add(resp);
	}

}
//---------------------------------------------------------------------------

void __fastcall TForm1::SendStartButClick(TObject *Sender)
{
	Console->Lines->Add("-> Send 'Start'");
	sframe[3]=1;
	sframe[4]=3;
	send_frame(0,2);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SendStopButClick(TObject *Sender)
{
	Console->Lines->Add("-> Send 'Stop'");
	send_frame(1,0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SendCleanButClick(TObject *Sender)
{
	Console->Lines->Add("-> Send 'Clean'");
	send_frame(0x56,0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ResetButClick(TObject *Sender)
{
	Console->Lines->Add("-> Send 'Reset'");
	send_frame(0xD3,0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ReadValuesButClick(TObject *Sender)
{
	int res;
	char line[256];
	float fval[10];
	Console->Lines->Add("-> Read Values");
	res=send_frame(3,0);
	if(res) return;    // Error
	if(iframe[3]<0x28){ // OK, but  No Values
		Console->Lines->Add("*** No Values (too fast or sensor off) ***");
		return;
	}
	fval[0]=get_float32(iframe+4); 	sprintf(line,"PM1.0    [ug/m^3]: %f",fval[0]); Console->Lines->Add(line);
	fval[1]=get_float32(iframe+8); 	sprintf(line,"PM2.5    [ug/m^3]: %f",fval[1]); Console->Lines->Add(line);
	fval[2]=get_float32(iframe+12); sprintf(line,"PM4.0    [ug/m^3]: %f",fval[2]); Console->Lines->Add(line);
	fval[3]=get_float32(iframe+16); sprintf(line,"PM10     [ug/m^3]: %f",fval[3]); Console->Lines->Add(line);
	fval[4]=get_float32(iframe+20); sprintf(line,"No PM0.5 [#/cm^3]: %f",fval[4]); Console->Lines->Add(line);
	fval[5]=get_float32(iframe+24); sprintf(line,"No PM1.0 [#/cm^3]: %f",fval[5]); Console->Lines->Add(line);
	fval[6]=get_float32(iframe+28); sprintf(line,"No PM2.5 [#/cm^3]: %f",fval[6]); Console->Lines->Add(line);
	fval[7]=get_float32(iframe+32); sprintf(line,"No PM4   [#/cm^3]: %f",fval[7]); Console->Lines->Add(line);
	fval[8]=get_float32(iframe+36); sprintf(line,"No PM10  [#/cm^3]: %f",fval[8]); Console->Lines->Add(line);
	fval[9]=get_float32(iframe+40); sprintf(line,"Typ. Size    [um]: %f",fval[9]); Console->Lines->Add(line);
	Console->Lines->Add("OK");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ButtonReadAutoCleaningClick(TObject *Sender)
{
    int res;
	char resp[256];
	unsigned long csecs;
	Console->Lines->Add("-> Read Auto Cleaning Interval");

	sframe[3]=0;    // Sub-Byte 0
	res=send_frame(0x80,1);
	if(!res){

		csecs=(iframe[4]<<24)+(iframe[5]<<16)+(iframe[6]<<8)+(iframe[7]);
		sprintf(resp,"Interval: %u secs (%.2f hr)",csecs,(float)csecs/3600.0);
		Console->Lines->Add(resp);
		sprintf(resp,"%u",csecs);
		EditSecs->Text=resp;
	}


}
//---------------------------------------------------------------------------

void __fastcall TForm1::ButtonSetAutoCleaningClick(TObject *Sender)
{
	char buf[256];
	int csecs;
	sprintf(buf,"%ls",EditSecs->Text.c_str());
	csecs=atol(buf);
	if(csecs<0 || csecs>(168*3600)){
		Console->Lines->Add("-> Error: must be 0 or <=168 hr");
		return;
	}
	sprintf(buf,"-> Set Cleaning Interval to %u secs (%.2f hr)",csecs,(float)csecs/3600.0);
	Console->Lines->Add(buf);
	sframe[3]=0;    // Sub-Byte 0
	sframe[4]=(csecs>>24)&255;
	sframe[5]=(csecs>>16)&255;
	sframe[6]=(csecs>>8)&255;
	sframe[7]=(csecs)&255;

	send_frame(0x80,5);
}
//---------------------------------------------------------------------------

