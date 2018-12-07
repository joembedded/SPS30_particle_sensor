//---------------------------------------------------------------------------

#ifndef mainwinH
#define mainwinH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>

//------------------- DEFs -----------------------
#define BMH 58  // Soviele Kanaele (Stretch aktiviert!)

//---------------------------------------------------------------------------


class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TButton *ExitBut;
	TButton *RunBut;
	TComboBox *ComCombo;
	TTimer *DrawTimer;
	TLabel *InfoLab;
	TButton *StopBut;
	TButton *ReadIDBut;
	TButton *SendStartBut;
	TButton *SendStopBut;
	TButton *ReadValuesBut;
	TButton *SendCleanBut;
	TButton *Button1;
	TButton *ResetBut;
	TMemo *Console;
	void __fastcall ExitButClick(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall RunButClick(TObject *Sender);
	void __fastcall StopButClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall ReadIDButClick(TObject *Sender);
	void __fastcall SendStartButClick(TObject *Sender);
	void __fastcall SendStopButClick(TObject *Sender);
	void __fastcall SendCleanButClick(TObject *Sender);
	void __fastcall ResetButClick(TObject *Sender);
	void __fastcall ReadValuesButClick(TObject *Sender);

private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
	void __fastcall Enabler(void);


};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
