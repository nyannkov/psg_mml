[English](README.md) | [日本語](README_ja.md)

YMZ294(PSG)とRaspberrypi Pico(以下rpi_pico)を使用します。
YMZ294とrpi_picoのピンは以下のように対応させます。

|YMZ294|rpi_pico|備考|
|--|--|--|
|/WR|GP15||
|/CS|GP15||
|A0|GP14||
|phiM|GP13|GP13は4MHzのPWM出力として使用|
|D0|GP28||
|D1|GP27||
|D2|GP26||
|D3|GP22||
|D4|GP21||
|D5|GP18||
|D6|GP19||
|D7|GP20||

YMZ294のphiM端子(マスタークロック入力)には、rpi_picoのPWM出力を入力しています。
PWMの周波数は4MHzとしたため、4/6端子(マスタークロック周波数選択)はHighにします。


**開発環境**
[Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)を使用します。

**ビルド**

以下のコマンドでビルドできます。

```bash
$ make build
```

