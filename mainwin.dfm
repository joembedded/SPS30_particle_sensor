object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'SPS30-Test V0.2'
  ClientHeight = 362
  ClientWidth = 484
  Color = clBtnFace
  Constraints.MinHeight = 400
  Constraints.MinWidth = 500
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnActivate = FormActivate
  DesignSize = (
    484
    362)
  PixelsPerInch = 96
  TextHeight = 13
  object InfoLab: TLabel
    Left = 8
    Top = 334
    Width = 83
    Height = 13
    Anchors = [akLeft, akBottom]
    Caption = 'Select COM-port:'
    ExplicitTop = 394
  end
  object ExitBut: TButton
    Left = 384
    Top = 329
    Width = 92
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Exit'
    TabOrder = 0
    OnClick = ExitButClick
  end
  object RunBut: TButton
    Left = 200
    Top = 329
    Width = 90
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Connect'
    TabOrder = 1
    OnClick = RunButClick
  end
  object ComCombo: TComboBox
    Left = 102
    Top = 331
    Width = 92
    Height = 21
    Style = csDropDownList
    Anchors = [akLeft, akBottom]
    TabOrder = 2
  end
  object StopBut: TButton
    Left = 296
    Top = 329
    Width = 75
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Disconnect'
    Enabled = False
    TabOrder = 3
    OnClick = StopButClick
  end
  object ReadIDBut: TButton
    Left = 384
    Top = 4
    Width = 92
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Read Sensor ID'
    Enabled = False
    TabOrder = 4
    OnClick = ReadIDButClick
  end
  object SendStartBut: TButton
    Left = 384
    Top = 35
    Width = 92
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Send '#39'Start'#39
    Enabled = False
    TabOrder = 5
    OnClick = SendStartButClick
  end
  object SendStopBut: TButton
    Left = 384
    Top = 66
    Width = 92
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Send '#39'Stop'#39
    Enabled = False
    TabOrder = 6
    OnClick = SendStopButClick
  end
  object ReadValuesBut: TButton
    Left = 384
    Top = 160
    Width = 92
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Read Values'
    Enabled = False
    TabOrder = 7
    OnClick = ReadValuesButClick
  end
  object SendCleanBut: TButton
    Left = 384
    Top = 97
    Width = 92
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Send '#39'Clean'#39
    Enabled = False
    TabOrder = 8
    OnClick = SendCleanButClick
  end
  object Button1: TButton
    Left = 8
    Top = 308
    Width = 468
    Height = 15
    Anchors = [akLeft, akRight, akBottom]
    Caption = 'Visit   JoEmbedded.de'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 9
    OnClick = Button1Click
  end
  object ResetBut: TButton
    Left = 384
    Top = 128
    Width = 92
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Send '#39'Reset'#39
    Enabled = False
    TabOrder = 10
    OnClick = ResetButClick
  end
  object Console: TMemo
    Left = 8
    Top = 8
    Width = 363
    Height = 294
    Anchors = [akLeft, akTop, akRight, akBottom]
    BevelInner = bvLowered
    BevelKind = bkTile
    Lines.Strings = (
      'SPS30 Particle Sensor V0.2 - 11.02.2018 (C) JoEmbedded.de'
      '===================================='
      '- SPS30 needs 3.3 or 5V UART (115kBd), leave Pin 4 (SEL) open'
      '- Written with Embarcadero C++ Builder 10.2'
      '')
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 11
  end
  object ButtonReadAutoCleaning: TButton
    Left = 384
    Top = 191
    Width = 92
    Height = 25
    Caption = 'Read Auto Clean'
    TabOrder = 12
    OnClick = ButtonReadAutoCleaningClick
  end
  object ButtonSetAutoCleaning: TButton
    Left = 384
    Top = 222
    Width = 92
    Height = 25
    Caption = 'Set Auto Clean'
    TabOrder = 13
    OnClick = ButtonSetAutoCleaningClick
  end
  object EditSecs: TEdit
    Left = 385
    Top = 245
    Width = 91
    Height = 21
    MaxLength = 16
    TabOrder = 14
    Text = '(in sec)'
  end
  object DrawTimer: TTimer
    Enabled = False
    Interval = 500
    Left = 264
    Top = 392
  end
end
