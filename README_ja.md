# psg_mml

[English](README.md) | [日本語](README_ja.md)

psg_mmlはPSG音源用のサウンドミドルウェアです。

## 特徴

 - MMLを使用したPSG制御。
 - MMLは和音の楽音発生命令に加え，ノイズの発生や，エンベロープによる音量制御，周波数変調などをサポート
 - 一つのPSGで異なるMMLセットを同時に演奏することが可能（例えばBGM演奏中に効果音を再生する，など）

# Examples

## [music_box](examples/music_box)

電源投入後に，メロディーを演奏するサンプルプログラムです。

 - Raspberrypi pico + YMZ294を使用した場合: [rpi_pico_ymz294](examples/music_box/boards/rpi_pico)

# Demo
準備中

# Details

## API reference

|API名|概要|
|--|--|
|[psg_mml_init](#psg_mml_init)|指定したスロットの初期化を行います。|
|[psg_mml_deinit](#psg_mml_deinit)|指定したスロットの終了処理を行います。|
|[psg_mml_periodic_control](#psg_mml_periodic_control)|周期的に実行してPSGを制御します。|
|[psg_mml_load_text](#psg_mml_load_text)|指定したスロットにMMLをロードします。|
|[psg_mml_decode](#psg_mml_decode)|ロードしたMMLをデコードします。|
|[psg_mml_play_start](#psg_mml_play_start)|ロードしたMMLの演奏を開始します。|
|[psg_mml_play_restart](#psg_mml_play_restart)|ロードしたMMLをはじめから演奏し直します。|
|[psg_mml_play_stop](#psg_mml_play_stop)|ロードしたMMLの演奏を停止します。|
|[psg_mml_get_play_state](#psg_mml_get_play_state)|ロードしたMMLの演奏状態を取得します。|

### psg_mml_init

指定したスロットの初期化を行います。

#### スロットについて

本ミドルウェアでは，スロットと呼ばれるオブジェクトにMMLを登録してPSGの制御を行います。
スロットは二つ存在し，それぞれ0番，1番と番号で管理されています。
単純にメロディを演奏する場合は，通常，0番のスロットを使用します。
スロットを二つ使用した場合の挙動については，設定値[PSG_MML_SHARE_SLOT0_DRIVER](#PSG_MML_SHARE_SLOT0_DRIVER)の説明を参照ください。
```c
psg_mml_t psg_mml_init(
    uint8_t slot
  , uint16_t tick_hz
  , void (*p_write)(uint8_t addr, uint8_t data)
  );
```
|パラメータ|説明|
|--|--|
|slot|初期化するスロット番号を0から1の範囲で指定します。|
|tick_hz|関数psg_mml_periodic_control()の呼び出しレートを指定します。単位はHzです。この値の逆数が演奏時間の分解能になります。|
|p_write|PSGにデータを書き込む関数へのポインタを指定します。なお，書き込み関数はpsg_mml_init()内で実行されるため，この関数を実行する前にPSGを初期化しておく必要があります。|

|戻り値|条件|
|--|--|
|PSG_MML_SUCCESS|関数が正常に終了した場合。|
|PSG_MML_OUT_OF_SLOT|範囲外のスロットが指定された場合。|

**NOTE:**
この関数を実行する際，psg_mml_periodic_control()の周期実行は停止する必要があります。


### psg_mml_deinit

指定したスロットの終了処理を行います。

```c
psg_mml_t psg_mml_deinit(
    uint8_t slot
    );
```
|パラメータ|説明|
|--|--|
|slot|終了処理を行うスロット番号を0から1の範囲で指定します。|

|戻り値|条件|
|--|--|
|PSG_MML_SUCCESS|関数が正常に終了した場合。|
|PSG_MML_OUT_OF_SLOT|範囲外のスロットが指定された場合。|

**NOTE:**
この関数の実行中は，関数psg_mml_periodic_control()の周期実行を停止する必要があります。


### psg_mml_periodic_control

MMLのデコード情報をFIFOから取得し，その情報をもとにPSGを制御します。
この関数は，初期化関数psg_mml_init()で指定したレート(tick_hz)で周期的に呼び出す必要があります。

```c
void psg_mml_periodic_control(void); 
```

**NOTE:**
psg_mml_periodic_control()実行中は，他のpsg_mmlの関数が実行されないようにする必要があります。

### psg_mml_load_text

演奏するMMLをスロットに登録します。

```c
psg_mml_t  psg_mml_load_text(
      uint8_t slot
    , const char *p_mml_text
    , uint16_t flags
    );
```

|パラメータ|説明|
|--|--|
|slot|MMLをロードするスロット番号を0から1の範囲で指定します。|
|p_mml_text|ロードするMMLテキストへのポインタを指定します。|
|flags|MMLデコードのオプションを指定します。|

|戻り値|条件|
|--|--|
|PSG_MML_SUCCESS|関数が正常に終了した場合。|
|PSG_MML_OUT_OF_SLOT|範囲外のスロットが指定された場合。|
|PSG_MML_NOT_INITIALIZED|指定したスロットが初期化されていない場合。|
|PSG_MML_TEXT_NULL|MMLテキストへのポインタがNULLである場合。|
|PSG_MML_TEXT_EMPTY|MMLテキストが空である場合。|
|PSG_MML_TEXT_OVER_LENGTH|MMLテキストが，読み込み可能なサイズを超えている場合。|

#### flagsについて

|bit|フィールド名|説明|
|--|--|--|
|15-1|-|予約|
|0|RH_LEN|R,Hコマンドで音の長さの指定を省略した場合のデフォルト値を切り替えます。<br> - 0: 音の長さのデフォルト値は4になります。<br> - 1: 音の長さのデフォルト値はLコマンドで指定した値になります。|


### psg_mml_decode

ロードしたMMLをデコードします。デコードした結果はFIFOを通してpsg_mml_periodic_control()に送信されます。
この関数は，一度にMMLのデコードは行わず，各チャンネルのMMLを１命令ずつデコードを行った後，結果をリターンするように動作します。
次にデコードを行うMMLテキストの位置はスロット内で保持されているため，この関数を繰り返し実行することで，MMLのデコードを逐一行うことができます。
MMLをすべてデコードし終えると，この関数はPSG_MML_DECODE_ENDをリターンします。

```c
psg_mml_t  psg_mml_decode(
    uint8_t slot
    );
```

|パラメータ|説明|
|--|--|
|slot|MMLをデコードするスロット番号を0から1の範囲で指定します。|

|戻り値|条件|
|--|--|
|PSG_MML_SUCCESS|ロードしたMMLのデコードが一部完了した場合。|
|PSG_MML_DECODE_END|ロードしたMMLのデコードがすべて完了した場合。|
|PSG_MML_DECODE_QUEUE_IS_FULL|FIFOに空きがなく，MMLのデコードを見送った場合。|
|PSG_MML_OUT_OF_SLOT|範囲外のスロットが指定された場合。|
|PSG_MML_NOT_INITIALIZED|指定したスロットが初期化されていない場合。|
|PSG_MML_INTERNAL_ERROR|内部エラーが発生した場合。|

### psg_mml_play_start

ロードしたMMLの演奏を開始します。

```c
psg_mml_t  psg_mml_play_start(
      uint8_t slot
    , uint16_t num_predecode
    );
```

|パラメータ|説明|
|--|--|
|slot|演奏を開始するスロット番号を0から1の範囲で指定します。|
|num_predecode|演奏を開始する前にデコードを行う回数を指定します。この関数を実行した直後に演奏が途切れてしまうような場合に，この値を調整します。|

|戻り値|条件|
|--|--|
|PSG_MML_SUCCESS|関数が正常に終了した場合。|
|PSG_MML_OUT_OF_SLOT|範囲外のスロットが指定された場合。|
|PSG_MML_NOT_INITIALIZED|指定したスロットが初期化されていない場合。|
|PSG_MML_INTERNAL_ERROR|内部エラーが発生した場合。|

### psg_mml_play_restart

ロードしたMMLをはじめから演奏し直します。

```c
psg_mml_t  psg_mml_play_restart(
      uint8_t slot
    , uint16_t num_predecode
    );
```

|パラメータ|説明|
|--|--|
|slot|演奏を開始するスロット番号を0から1の範囲で指定します。|
|num_predecode|演奏を開始する前にデコードを行う回数を指定します。この関数を実行した直後に演奏が途切れてしまうような場合に，この値を調整します。|

|戻り値|条件|
|--|--|
|PSG_MML_SUCCESS|関数が正常に終了した場合。|
|PSG_MML_OUT_OF_SLOT|範囲外のスロットが指定された場合。|
|PSG_MML_NOT_INITIALIZED|指定したスロットが初期化されていない場合。|
|PSG_MML_INTERNAL_ERROR|内部エラーが発生した場合。|


### psg_mml_play_stop

ロードしたMMLの演奏を停止します。

```c
psg_mml_t  psg_mml_play_stop(
    uint8_t slot
    );
```

|パラメータ|説明|
|--|--|
|slot|演奏を停止するスロット番号を0から1の範囲で指定します。|

|戻り値|条件|
|--|--|
|PSG_MML_SUCCESS|関数が正常に終了した場合。|
|PSG_MML_OUT_OF_SLOT|範囲外のスロットが指定された場合。|
|PSG_MML_NOT_INITIALIZED|指定したスロットが初期化されていない場合。|


### psg_mml_get_play_state

ロードしたMMLの演奏状態を取得します。演奏状態は引数p_outが指す変数に格納されます。

```c
psg_mml_t  psg_mml_get_play_state(
      uint8_t slot
    , PSG_MML_PLAY_STATE_t *p_out
    );
```

|パラメータ|説明|
|--|--|
|slot|演奏状態を取得するスロット番号を0から1の範囲で指定します。|
|p_out|現在の演奏状態を格納する変数へのポインタを指定します。|

|戻り値|条件|
|--|--|
|PSG_MML_SUCCESS|関数が正常に終了した場合。|
|PSG_MML_OUT_OF_SLOT|範囲外のスロットが指定された場合。|
|PSG_MML_NOT_INITIALIZED|指定したスロットが初期化されていない場合。|

#### PSG_MML_PLAY_STATE_t

|列挙子|説明|
|--|--|
|E_PSG_MML_PLAY_STATE_STOP|演奏停止中|
|E_PSG_MML_PLAY_STATE_PLAY|演奏中|
|E_PSG_MML_PLAY_STATE_END|演奏終了|


## Configuration

以下の設定定数およびマクロをpsg_mml_conf.h内に記述することができます。
このヘッダのテンプレートとして，psg_mml_conf_template.hを用意しています。
環境に合わせてこのテンプレートの内容を編集し，ファイル名をpsg_mml_conf.hにリネームして使用ください。
ここでは各設定値およびマクロについて記載します。

### PSG_MML_FIFO_SCALE

MMLのデコード情報を格納するFIFOの長さを変更できます。この定数のデフォルトは8です。
各楽音チャンネルのFIFOには，少なくともこの値で指定した個数分の音符および休符を格納することができます。

### PSG_MML_SHARE_SLOT0_DRIVER

スロット0, 1の動作をtrue/falseで設定します。この定数のデフォルトはtrueです。各値におけるスロット0, 1の動作は以下のとおりです。

**trueの場合**

スロット0, 1を連携させて演奏します。一つのPSGをスロット0，1で共有する場合は，この設定値でビルドを実行します。
この設定では，3つあるPSGの楽音チャンネル(チャンネルA, BおよびC)をスロット1に優先的に割り当てて演奏を行います。
例えばスロット0で3和音のBGMを演奏している最中に，スロット1で単音の効果音を発生させるような場合，楽音チャンネルCをスロット1に割り当て，
残りの楽音チャンネルA, Bをスロット0に割り当てて演奏を行います。（一時的にスロット0は2和音で演奏することになります。)
カンマで区切った各パートのMMLは，スロット0の場合は左から順に楽音チャンネルA, B, Cが割り当てられます。
逆に，スロット1ではカンマで区切った各パートのMMLは，楽音チャンネルC, B, Aに割り当てられます。
そのため，スロット０でBGMを演奏する場合，一時的に音が消えて問題ないパートをカンマ区切りの右側に記述することを推奨します。

**falseの場合**
 
スロット0, 1を連携させず個別に演奏を行います。スロット0および1に個別にPSGを割り当てるような場合は，このモードを選択します。



### PSG_MML_DISABLE_PERIODIC_CONTROL()

psg_mml_periodic_control()の実行開始を禁止する区間に入った際に，このマクロが実行されます。
psg_mml_periodic_control()の実行が，他の関数とは別スレッドで動作している場合，このマクロに実行開始を抑止する処理を記述する必要があります。
このマクロのデフォルトは空です。
なお，このマクロを実行する関数は，後述のマクロPSG_MML_ENABLE_PERIODIC_CONTROL()を実行してから終了します。

### PSG_MML_ENABLE_PERIODIC_CONTROL()

psg_mml_periodic_control()の実行開始を禁止する区間から抜ける際に，このマクロが実行されます。このマクロのデフォルトは空です。
PSG_MML_DISABLE_PERIODIC_CONTROL()に抑止処理を記述した場合，このマクロに抑止解除処理を記述する必要があります。

### PSG_MML_HOOK_START_PERIODIC_CONTROL()

psg_mml_periodic_control()関数開始時に，このマクロ関数が実行されます。このマクロ関数のデフォルトは空です。

### PSG_MML_HOOK_END_PERIODIC_CONTROL()

psg_mml_periodic_control()関数終了時に，このマクロ関数が実行されます。このマクロ関数のデフォルトは空です。

### PSG_MML_ASSERT(TF)

デバッグ用のマクロです。このマクロのデフォルトは空です。 このマクロはローカル関数内の条件検証で実行されます。

# MML instructions

ここではpsg_mmlで使用するMMLの各コマンドについて記述します。

## 基本コマンド

### A-G [&lt;accidental&gt;] [&lt;length&gt;] [&lt;dot&gt;]

指定した音名の音を出力します。

|Values|Description|
|--|--|
|&lt;accidental&gt;|変化記号を「#」，「+」，「-」で指定します。「#」，「+」は半音上げ，「-」は半音下げに対応します。|
|&lt;length&gt;|音の長さを0から64の範囲で指定します。1は全音符，4は4分音符を表します。&lt;length&gt;を省略した場合は，音の長さはLコマンドで指定した値になります。音の長さとして0を指定することも可能です。この場合，音の発生は行いません。この値は主に，スラー(&)で音同士をつなげた際の終端音に対して使用します。|
|&lt;dot&gt;|付点を「.」で指定します。付点を1つ記述すると音の長さは1.5倍(=1+0.5)になります。2つ記述すると音の長さは1.75倍(=1+0.5+0.25)になります。付点は最大10個記述できます。|

**Example:**
```
C#2.
```

### N &lt;note-number&gt; [&lt;dot&gt;]

ノートナンバーを指定して音を出力します。音の長さはLコマンドで指定した値に従います。

|Values|Description|
|--|--|
|&lt;note-number&gt;|ノートナンバーの値を0から95の範囲で指定します。例えば，N36はO4Cに，N52はO5Eに対応します。ただし，N0はO1Cではなく休符として扱います。|
|&lt;dot&gt;|付点を指定します。付点の効果はA-Gの場合と同様です。|

**Example:**
```
O4CEG. N36N40N43.
```

### R [&lt;length&gt;] [&lt;dot&gt;]

休符を挿入します。

|Values|Description|
|--|--|
|&lt;length&gt;|休符の長さを1から64の範囲で指定します。1は全休符，4は4分休符を表します。この値を省略した場合の休符の長さは，以下のように関数psg_mml_load_mml()の引数flagsの値によって決まります。<br> - flagsの0bit目が0の場合: 休符の長さは4(4分休符)になります。<br> - flagsの0bit目が1の場合: 休符の長さはLコマンドで指定した値になります。|
|&lt;dot&gt;|付点を指定します。付点の動作はA-Gの場合と同様です。|

**Example:**
```
CE.R8G
```

### H [&lt;length&gt;] [&lt;dot&gt;]

Iコマンドで設定した周波数のノイズ音を出力します。

|Values|Description|
|--|--|
|&lt;length&gt;|ノイズの長さを1から64の範囲で指定します。この値を省略した場合のノイズの長さは，Rコマンドと同様に関数psg_mml_load_mml()の引数flagsの値によって決まります。|
|&lt;dot&gt;|付点を指定します。付点の動作はA-Gの場合と同様です。|

**Example:**
```
HR8H16R16HR HR8H16R16HR
```

### T &lt;tempo&gt;

テンポを設定します。テンポのデフォルトは120 bpmです。

|Values|Description|
|--|--|
|&lt;tempo&gt;|テンポの値を32から255の範囲で指定します。複数の楽音チャンネルを使用して演奏する場合，テンポの設定は，各チャンネルのMMLに記述する必要があります。|

**Example:**
```
T120CDE2T100CDE2,
T120EGB2T100EGB2
```

### L &lt;length&gt; [&lt;dot&gt;]

A-Gコマンドで&lt;length&gt;を省略した場合の音の長さを設定します。この値のデフォルトは4です。
この値は，psg_mml_load_mml()関数の引数flagsの値によってRおよびHコマンドにも適用することが可能です。

|Values|Description|
|--|--|
|&lt;length&gt;|1-64で音の長さを指定します。1は全音符，4は4分音符を表します。|
|&lt;dot&gt;|付点を指定します。付点の動作はA-Gの場合と同様です。|

**Example:**
```
L16CDE
```

### V &lt;volume&gt;

トーンおよびノイズの音量を設定します。音量のデフォルトは15です。
なお，VコマンドとSコマンドは排他的に動作し，音量設定は後から実行したコマンドに従います。
|Values|Description|
|--|--|
|&lt;volume&gt;|音量を0から15の範囲で指定します。|

**Example:**
```
V15A V13A V10A
```

### S &lt;shape&gt;

エンベロープ発生器による音量制御に切り替えます。この値のデフォルトはOFFです。
このコマンドを実行すると，エンベロープ発生器による音量制御がONになります。
この音量制御をOFFしたい場合は，Vコマンドを実行してください。

|Values|Description|
|--|--|
|&lt;shape&gt;|エンベロープの形状を0から15の範囲で指定します。各数値に対するエンベロープの形状についてはPSGのデータシートを参照ください。|

**Example:**
```
L4S0M3000CDER4
L4S2M3000CDER4
V15CDER4
```

### M &lt;frequency&gt;

エンベロープ周波数を指定します。エンベロープ周波数のデフォルトは0です。

|Values|Description|
|--|--|
|&lt;frequency&gt;|エンベロープ周波数(EP)を0から65535の範囲で指定します。|

**Example:**
```
L4
S0M5000CDER4
S0M1000CDER4
```

### O &lt;octave&gt;

A-Gで指定する音のオクターブを指定します。オクターブのデフォルトは4です。

|Values|Description|
|--|--|
|&lt;octave&gt;|オクターブを1から8の範囲で指定します。|

**Example:**
```
O4ABO5C
```

### Q &lt;gate-time&gt;

音符のゲートタイムを指定します。ゲートタイムのデフォルト値は8(8/8=100%)です。

|Values|Description|
|--|--|
|&lt;gate-time&gt;|ゲートタイムを1から8の範囲で指定できます。例えば，ゲートタイムを3とした場合，音符の発音時間は3/8となり，残りの5/8はミュートになります。|

**Example:**
```
L4Q3CDER4
```

### I &lt;frequency&gt;

Hコマンドで発生するノイズの周波数(NP)を設定します。ノイズの周波数のデフォルトは16です。

|Values|Description|
|--|--|
|&lt;frequency&gt;|ノイズの周波数を0から31の範囲で指定します。|

**Example:**
```
I0H4 I8H4
```

### &lt;

Oコマンドで指定したオクターブを一つ下げます。

**Example:**
```
L4O5C<BA
```

### &gt;

Oコマンドで指定したオクターブを一つ上げます。

**Example:**
```
L4AB>C
```

### ,
各パートのMMLをカンマで連結することができます。PSGにはチャンネルが3つあるため，最大3つのMMLを連結できます。

**Example:**
```
T120L4O4CEG,
T120L4O4EGB,
T120L4O4GB>D
```

### & [&lt;octave-settings&gt;]

A-Gコマンド同士を&で連結することで，スラーやタイのような演奏を行えます。

|Values|Description|
|--|--|
|&lt;octave-settings&gt;|Oコマンド，&gt;および&lt;を使用してオクターブ設定を行うことができます。ここで設定したオクターブ値はスラー，タイによる演奏後も継続します。|

**Example:**
```
A2R2 A4&A4R2 A2&>A0R2 A2&<A0R2
```

### [ [&lt;loop-number&gt;] ... ]

[]内のMMLをループ再生します。ループは5段までネスティングできます。6段目以降のループ記号は無視されます。

|Values|Description|
|--|--|
|&lt;loop-number&gt;|ループ回数を0から255の範囲で指定します。ただし，0を指定した場合は無限ループになります。この値を省略した場合，ループ回数は1になります。|

**Example:**
```
[3
  [2
    L4CDER4
  ]
  L4GEDCDEDR4
]
```

## ソフトウェアエンベロープ発生器

psg_mmlでは，PSG内蔵のエンベロープ発生器とは別に，ソフトウェアエンベロープによる音量制御もサポートします。
この機能は，SコマンドではなくVコマンドで音量を制御している場合にのみ有効化できます。
エンベロープの形状は6つのパラメータ(AHDSFR方式)で設定します。
このエンベロープによる音量制御は各チャンネルで個別に設定することが可能です。

### $E &lt;enabled&gt;

ソフトウェアエンベロープによる音量制御のON／OFFを設定します。この値のデフォルトはOFFです。

|Values|Description|
|--|--|
|&lt;enabled&gt;|0から1の範囲で設定します。0はOFF，1はONに対応します。|

### $A &lt;attack&gt;

音の立ち上がり時間(音量0からVコマンドで設定した音量までの到達時間)を指定します。

|Values|Description|
|--|--|
|&lt;attack&gt;|0から10000の範囲で指定します。単位はmsです。0を指定した場合は音の立ち上がり処理は行わず，Vコマンドで設定した音量で音を出力します。|

### $H &lt;hold&gt;

Attack後の音量レベルの保持時間を設定します。

|Values|Description|
|--|--|
|&lt;hold&gt;|0から10000の範囲で指定します。単位はmsです。0を指定した場合，直ちにDecay処理に移ります。|

### $D &lt;decay&gt;

Hold後, 音量がSustain値に到達するまでの時間を設定します。

|Values|Description|
|--|--|
|&lt;decay&gt;|0から10000の範囲で指定します。単位はmsです。0を指定した場合は直ちに音量をSustain値に設定し，Fade処理に移ります。|

### $S &lt;sustain&gt;

Decay処理の目標音量レベルを設定します。

|Values|Description|
|--|--|
|&lt;sustain&gt;|0から100の範囲で指定します。単位は%です。Vコマンドで指定した音量に対する百分率に対応します。|

### $F &lt;fade&gt;

Decay処理完了後，音量がSustain値から0になるまでの時間を指定します。

|Values|Description|
|--|--|
|&lt;fade&gt;|0から10000の範囲で指定します。単位はmsです。ただし，0を指定した場合は，音量はSustain値を保持し続けます。|

### $R &lt;release&gt;

ノートオフ後に，出力音量を現在値から0にするまでの時間を指定します。

|Values|Description|
|--|--|
|&lt;release&gt;|0から10000の範囲で指定します。単位はmsです。ただし，0を指定した場合は音量を直ちに0にします。|

## ソフトウェアLFO

psg_mmlには，ソフトウェアLFOを搭載しています。この機能を有効にすると，ビブラートを効かせた音を出力することができます。

### $M &lt;mode&gt;

ソフトウェアLFOのON／OFFを設定します。この値のデフォルトはOFFです。

|Values|Description|
|--|--|
|&lt;mode&gt;|ソフトウェアLFOモードを0から1の範囲で設定します。0はOFF，1はON(三角波による変調)に対応します。|

### $J &lt;depth&gt;

変調深さを設定します。変調深さのデフォルトは0です。

|Values|Description|
|--|--|
|&lt;depth&gt;|変調深さを0から360の範囲で指定します。変調深さの値をnとすると，音の周波数は変調関数により，2^(-n/360)倍から2^(n/360)倍まで変化します。|

### $L &lt;low-frequency&gt;

変調周波数を設定します。変調周波数のデフォルトは40( = 4.0 Hz)です。

|Values|Description|
|--|--|
|&lt;low-frequency&gt;|変調周波数を0から200の範囲で指定します。単位は0.1 Hzです。|

### $T &lt;delay&gt; [&lt;dot&gt;]

音の出力を開始してからLFOが動作するまでの時間を指定します。この値のデフォルトは0です。

|Values|Description|
|--|--|
|&lt;delay&gt;|LFOのディレイ時間を0から64の範囲で指定します。delayが0以外の場合，Lコマンドと同じ計算に従いディレイ時間を決定します。例えばdelayを4とすると，発音を開始してから4分音符１つ分の時間が経過した後にLFOの動作を開始します。delayが0の場合はディレイ時間は0になります。|
|&lt;dot&gt;|付点を指定します。付点の動作はA-Gの場合と同様です。|

## その他

### $B &lt;bias&gt;

出力する音の周波数にバイアスをかけます。バイアスのデフォルト値は0です。

|Values|Description|
|--|--|
|&lt;bias&gt;|バイアスを(-2880)から2880の範囲で指定します。バイアスの値をnとすると，出力する音の周波数は2^(n/360)倍した値になります。|

### $P &lt;pitchbend-level&gt;

出力した音の周波数を，出力が停止するまでの間，pitchbend-levelで指定した値までなめらかに増加または減少させます。この値のデフォルトは0です。

|Values|Description|
|--|--|
|&lt;pitchbend-level&gt;|ピッチベントレベルを(-2880)から2880の範囲で指定します。指定した値をnとすると，出力する音の周波数は最終的に2^(n/360)倍した値までなめらかに変化します。|


# License

MITライセンスです。

