# psg_mml

[English](README.md) | [日本語](README_ja.md)

psg_mml is a sound middleware for PSG sound source. 

## Features

 - PSG control using MML.
 - MML supports not only chord music generation commands, but also noise generation, volume control by envelope, frequency modulation, etc.
 - A single PSG can play different sets of MML simultaneously (e.g., play sound effects while background music is playing).

# Examples

## [music_box](examples/music_box)

This is a sample program that plays a melody after turning on the power.

  - When using Raspberrypi pico + YMZ294: [rpi_pico_ymz294](examples/music_box/boards/rpi_pico)

# Demo
Under preparation.


# Details
## API reference

|API Name|Brief|
|--|--|
|[psg_mml_init](#psg_mml_init)|Initializes the specified slot. |
|[psg_mml_deinit](#psg_mml_deinit)|Terminates the specified slot.|
|[psg_mml_load_text](#psg_mml_load_text)|Loads MML into the specified slot. |
|[psg_mml_decode](#psg_mml_decode)|Decodes the loaded MML. |
|[psg_mml_periodic_control](#psg_mml_periodic_control)|Controls PSG by reading the decoding information of the loaded MML. |
|[psg_mml_play_start](#psg_mml_play_start)|Start playing the loaded MML. |
|[psg_mml_play_restart](#psg_mml_play_restart)|Restarts the loaded MML from the beginning. |
|[psg_mml_play_stop](#psg_mml_play_stop)|Stop playing the loaded MML. |
|[psg_mml_get_play_state](#psg_mml_get_play_state)|Gets the play state of the loaded MML. |

### psg_mml_init

Initializes the specified slot.

#### About slots

This middleware controls PSG by registering MML in objects called slots.
There are two slots, which are numbered 0 and 1 respectively.
Slot 0 is usually used to simply play a melody.
For the behavior when using two slots, see the description of the setting value [PSG_MML_SHARE_SLOT0_DRIVER](#PSG_MML_SHARE_SLOT0_DRIVER).

```c
psg_mml_t psg_mml_init(
    uint8_t slot
  , uint16_t tick_hz
  , void (*p_write)(uint8_t addr, uint8_t data)
  );
```
|Parameters|Description|
|--|--|
|slot|Specifies the slot number to initialize, in the range of 0 to 1.|
|tick_hz|Specifies the call rate of the function psg_mml_periodic_control(). The unit is Hz. The reciprocal of this value is the performance time resolution.|
|p_write|Specifies a pointer to a function that writes data to the PSG. Note that the write function is executed within psg_mml_init(), so the PSG must be initialized before executing this function.|

|Return values|Conditions|
|--|--|
|PSG_MML_SUCCESS|The function completed successfully.|
|PSG_MML_OUT_OF_SLOT|An out-of-range slot is specified.|


This middleware provides two slots for loading MML (slots 0 and 1) so that different music data can be played simultaneously.
The behavior of simultaneous MML playback using these slots depends on the value of the setting [PSG_MML_SHARE_SLOT0_DRIVER] (#PSG_MML_SHARE_SLOT0_DRIVER).

**NOTE:**
Periodic execution of the function psg_mml_periodic_control() must be stopped while this function is running.


### psg_mml_deinit

Terminates the specified slot.

**NOTE:**
Periodic execution of the function psg_mml_periodic_control() must be stopped while this function is running.

```c
psg_mml_t psg_mml_deinit(
    uint8_t slot
    );
```
|Parameters|Description|
|--|--|
|slot|Specify the slot number to perform termination processing in the range of 0 to 1.|

|Return values|Conditions|
|--|--|
|PSG_MML_SUCCESS|The function completed successfully.|
|PSG_MML_OUT_OF_SLOT|An out-of-range slot is specified.|

### psg_mml_periodic_control

Acquires MML decoding information from FIFO and controls PSG based on that information.
This function should be called periodically at the rate of tick_hz after executing the initialization function psg_mml_init().

```c
void psg_mml_periodic_control(void); 
```

**NOTE:**
Other psg_mml functions must not be executed while psg_mml_periodic_control() is being executed.

### psg_mml_load_text

Load the MML to play.

```c
psg_mml_t  psg_mml_load_text(
      uint8_t slot
    , const char *p_mml_text
    , uint16_t flags
    );
```

|Parameters|Description|
|--|--|
|slot|Specify the slot number from 0 to 1 to load the MML.|
|p_mml_text|Specifies a pointer to the MML text to load.|
|flags|Specifies options for MML decoding.|

|Return values|Conditions|
|--|--|
|PSG_MML_SUCCESS|The function completed successfully.|
|PSG_MML_OUT_OF_SLOT|An out-of-range slot is specified.|
|PSG_MML_NOT_INITIALIZED|The specified slot has not been initialized.|
|PSG_MML_TEXT_NULL|The pointer to MML text is NULL.|
|PSG_MML_TEXT_EMPTY|The MML text is empty.|
|PSG_MML_TEXT_OVER_LENGTH|The MML text exceeds the readable size.|

#### About flags

|bit|Field|Description|
|--|--|--|
|15-1|-|Reserved.|
|0|RH_LEN|Switches the default value when the R,H command omits the note length specification. <br> - 0: The default value for note length is 4. <br> - 1: The default note length is the value specified with the L command.|

### psg_mml_decode

Decode the loaded MML. The decoded result is sent to psg_mml_periodic_control() through FIFO.
This function does not decode the MML at once, but decodes the MML of each channel one instruction at a time and then returns the result.
Since the position of the MML text to be decoded next is held in the slot, MML decoding can be performed one by one by repeatedly executing this function.
After decoding all MML, this function returns PSG_MML_DECODE_END.

```c
psg_mml_t  psg_mml_decode(
    uint8_t slot
    );
```

|Parameters|Description|
|--|--|
|slot|Specifies the slot number from 0 to 1 in which to decode MML.|

|Return values|Conditions|
|--|--|
|PSG_MML_SUCCESS|Decoding of loaded MML is partially complete.|
|PSG_MML_DECODE_END|All decoding of loaded MML is complete.|
|PSG_MML_DECODE_QUEUE_IS_FULL|No room in the FIFO, MML decoding was not performed.|
|PSG_MML_OUT_OF_SLOT|An out-of-range slot is specified.|
|PSG_MML_NOT_INITIALIZED|The specified slot has not been initialized.|
|PSG_MML_INTERNAL_ERROR|An internal error occurred.|

### psg_mml_play_start

Starts playing the loaded MML.

```c
psg_mml_t  psg_mml_play_start(
      uint8_t slot
    , uint16_t num_predecode
    );
```

|Parameters|Description|
|--|--|
|slot|Specifies the slot number from 0 to 1 where the performance will start.|
|num_predecode|Specifies the number of times to decode before starting the performance. Adjust this value if the performance is interrupted immediately after this function is executed.|

|Return values|Conditions|
|--|--|
|PSG_MML_SUCCESS|The function completed successfully.|
|PSG_MML_OUT_OF_SLOT|An out-of-range slot is specified.|
|PSG_MML_NOT_INITIALIZED|The specified slot has not been initialized.|
|PSG_MML_INTERNAL_ERROR|n internal error occurred.|

### psg_mml_play_restart

Play the loaded MML again from the beginning.

```c
psg_mml_t  psg_mml_play_restart(
      uint8_t slot
    , uint16_t num_predecode
    );
```

|Parameters|Description|
|--|--|
|slot|Specifies the slot number from 0 to 1 where the performance will start.|
|num_predecode|Specifies the number of times to decode before starting the performance. Adjust this value if the performance is interrupted immediately after this function is executed.

|Return values|Conditions|
|--|--|
|PSG_MML_SUCCESS|The function completed successfully.|
|PSG_MML_OUT_OF_SLOT|An out-of-range slot is specified.|
|PSG_MML_NOT_INITIALIZED|The specified slot has not been initialized.|
|PSG_MML_INTERNAL_ERROR|n internal error occurred.|


### psg_mml_play_stop

Stops MML performance.

```c
psg_mml_t  psg_mml_play_stop(
    uint8_t slot
    );
```

|Parameters|Description|
|--|--|
|slot|Specifies the slot number in the range of 0 to 1 where the performance will stop.|

|Return values|Conditions|
|--|--|
|PSG_MML_SUCCESS|The function completed successfully.|
|PSG_MML_OUT_OF_SLOT|An out-of-range slot is specified.|
|PSG_MML_NOT_INITIALIZED|The specified slot has not been initialized.|

### psg_mml_get_play_state

Gets the playing status of MML. The playing state is stored in the variable pointed to by parameter p_out.

```c
psg_mml_t  psg_mml_get_play_state(
      uint8_t slot
    , PSG_MML_PLAY_STATE_t *p_out
    );
```

|Parameters|Description|
|--|--|
|slot|Specify the slot number from 0 to 1 to get the playing status.|
|p_out|Specifies a pointer to a variable that stores the current playing state.|

|Return values|Conditions|
|--|--|
|PSG_MML_SUCCESS|The function completed successfully.|
|PSG_MML_OUT_OF_SLOT|An out-of-range slot is specified.|
|PSG_MML_NOT_INITIALIZED|The specified slot has not been initialized.|

#### PSG_MML_PLAY_STATE_t

|Enumerator|Description|
|--|--|
|E_PSG_MML_PLAY_STATE_STOP|Not playing.|
|E_PSG_MML_PLAY_STATE_PLAY|Playing.|
|E_PSG_MML_PLAY_STATE_END|Finished playing.|


## Configuration

The following configuration constants and macros can be written in psg_mml_conf.h.
psg_mml_conf_template.h is provided as a template for this header.
Please edit the contents of this template according to your environment and rename the file name to psg_mml_conf.h.
Each setting value and macro are described here.

### PSG_MML_FIFO_SCALE

You can change the length of the FIFO that stores the MML decoding information. The default for this constant is 8.
The FIFO of each music channel can contain at least the number of notes and rests specified by this value.

### PSG_MML_SHARE_SLOT0_DRIVER

Sets the behavior of slots 0 and 1 as true/false. The default for this constant is true. The behavior of slots 0 and 1 for each value is as follows

**true:**

Slots 0 and 1 are played in conjunction. If a single PSG is shared by slots 0 and 1, the build is performed with this setting value.
With this setting, the three PSG music channels (channels A, B and C) are preferentially assigned to slot 1 for playing.
For example, if you are playing background music with 3 chords in slot 0 and want to generate a single note sound effect in slot 1, you would assign music channel C to slot 1 and play the rest of the music channels A, B, and C in slot 0.
The remaining music channels A and B are assigned to slot 0 (temporarily, slot 0 is assigned to 2 chords, and slot 1 is assigned to 2 chords). （) The remaining music channels A and B are assigned to slot 0 and played.
The MML for each comma-separated part is assigned to music channels A, B, and C in order from left to right in slot 0.
Conversely, in slot 1, the MML for each part separated by commas is assigned to music channels C, B, and A.
Therefore, when playing background music in slot 0, it is recommended to write the parts that can be temporarily silenced on the right side of the comma delimiter.

**false:**
 
Slots 0 and 1 are not linked and are played individually. Select this mode if you want to assign PSGs to slots 0 and 1 individually.


### PSG_MML_DISABLE_PERIODIC_CONTROL()

This macro is executed when entering the section where the start of execution of psg_mml_periodic_control() is prohibited.
If the execution of psg_mml_periodic_control() is running in a different thread than other functions, it is necessary to code processing to suppress the start of execution in this macro.
The default for this macro is empty.

Note that the function that executes this macro ends after executing the macro PSG_MML_ENABLE_PERIODIC_CONTROL() described later.
Therefore, it does not continue to suppress the psg_mml_periodic_control() function after the function processing ends.

### PSG_MML_ENABLE_PERIODIC_CONTROL()

This macro is executed when exiting the section that prohibits the start of execution of psg_mml_periodic_control(). The default for this macro is empty.
If the suppression processing is written in the macro PSG_MML_DISABLE_PERIODIC_CONTROL(), it is necessary to write the suppression release processing in this macro.

### PSG_MML_HOOK_START_PERIODIC_CONTROL()

This macro function is executed when the psg_mml_periodic_control() function starts executing. The default for this macro function is empty.

### PSG_MML_HOOK_END_PERIODIC_CONTROL()

This macro function is executed at the end of execution of the psg_mml_periodic_control() function. The default for this macro function is empty.

### PSG_MML_ASSERT(TF)

Macro for debugging. The default for this macro is empty. This macro runs on conditional validation within a local function.


# MML instructions

This section describes each MML command used in psg_mml.

## Basic command

### A-G [&lt;accidental&gt;] [&lt;length&gt;] [&lt;dot&gt;]

Outputs the sound of the specified note name.

|Values|Description|
|--|--|
|&lt;accidental&gt;|Specify the accidental symbols with "#", "+", and "-". "#" and "+" correspond to semitone up, and "-" correspond to semitone down.|
|&lt;length&gt;|Specify the length of the sound in the range from 0 to 64. 1 represents a whole note, 4 represents a quarter note. If you omit &lt;length&gt;, the note length will be the value specified by the L command. It is also possible to specify 0 as the length of the note. In this case, no sound is generated. This value is primarily used for the final tone of a slur (&) string.|
|&lt;dot&gt;|Specify the dot with ".". If you write one dot, the length of the sound will be 1.5 times (=1+0.5). If you write two, the length of the sound will be 1.75 times (= 1 + 0.5 + 0.25). Up to 10 dots can be described.|

**Example:**
```
C#2.
```

### N &lt;note-number&gt; [&lt;dot&gt;]

Specify the note number and output the sound. The note length follows the value specified with the L command.

|Values|Description|
|--|--|
|&lt;note-number&gt;|Specify the note number value in the range from 0 to 95. For example, N36 corresponds to O4C and N52 corresponds to O5E. However, N0 is treated as a rest, not O1C.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|

**Example:**
```
O4CEG. N36N40N43.
```

### R [&lt;length&gt;] [&lt;dot&gt;]

Insert a rest.

|Values|Description|
|--|--|
|&lt;length&gt;|Specify the length of the rest in the range of 1 to 64. 1 represents a whole rest, 4 represents a quarter note rest. If this value is omitted, the rest length is determined by the value of the argument flags of the function psg_mml_load_mml() as follows:<br> - If the 0th bit of flags is 0, the rest length is 4 (quarter rest).<br> - If the 0th bit of flags is 1, the length of the rest will be the value specified by the L command.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|

**Example:**
```
CE.R8G
```

### H [&lt;length&gt;] [&lt;dot&gt;]

Outputs a noise sound with the frequency set by the I command.

|Values|Description|
|--|--|
|&lt;length&gt;|Specifies the length of the noise in the range 1 to 64. If this value is omitted, the length of the noise is determined by the value of the argument flags of the function psg_mml_load_mml(), just like the R command.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|

**Example:**
```
HR8H16R16HR HR8H16R16HR
```

### T &lt;tempo&gt;

Sets the tempo. Tempo defaults to 120 bpm.

|Values|Description|
|--|--|
|&lt;tempo&gt;|Specify a tempo value in the range 32 to 255. When performing using multiple tone channels, tempo settings must be written in the MML for each channel.|

**Example:**
```
T120CDE2T100CDE2,
T120EGB2T100EGB2
```

### L &lt;length&gt; [&lt;dot&gt;]

Sets the length of the sound when &lt;length&gt; is omitted in the A-G command. This value defaults to 4.
This value can also be applied to the R and H commands depending on the value of the argument flags of the psg_mml_load_mml() function.

|Values|Description|
|--|--|
|&lt;length&gt;|Specify the length of the sound from 1-64. 1 represents a whole note, 4 represents a quarter note.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|

**Example:**
```
L16CDE
```

### V &lt;volume&gt;

Sets the volume of tones and noises. Volume defaults to 15.
Note that the V command and S command operate exclusively, and the volume setting follows the command executed later.

|Values|Description|
|--|--|
|&lt;volume&gt;|Specifies the volume from 0 to 15.|

**Example:**
```
V15A V13A V10A
```

### S &lt;shape&gt;

Switch to volume control with an envelope generator. This value defaults to OFF.
Executing this command turns on the volume control by the envelope generator.
If you want to turn off this volume control, execute the V command.

|Values|Description|
|--|--|
|&lt;shape&gt;|Specifies the shape of the envelope in the range 0-15. Please refer to the PSG datasheet for the shape of the envelope for each value.|

**Example:**
```
L4S0M3000CDER4
L4S2M3000CDER4
V15CDER4
```

### M &lt;frequency&gt;

Specifies the envelope frequency. Envelope frequency defaults to 0.

|Values|Description|
|--|--|
|&lt;frequency&gt;|Specifies the envelope frequency (EP) in the range 0 to 65535. |

**Example:**
```
L4
S0M5000CDER4
S0M1000CDER4
```

### O &lt;octave&gt;

Specifies the octave of the note specified by A-G. Octave defaults to 4.

|Values|Description|
|--|--|
|&lt;octave&gt;|Specify an octave in the range 1 to 8.|

**Example:**
```
O4ABO5C
```

### Q &lt;gate-time&gt;

Specifies the note gate time. The default value is 8 (8/8=100%).

|Values|Description|
|--|--|
|&lt;gate-time&gt;|Specify the gate time in the range of 1 to 8. For example, if the gate time is set to 3, the note duration will be 3/8 and the remaining 5/8 will be muted.|

**Example:**
```
L4Q3CDER4
```

### I &lt;frequency&gt;

Sets the noise frequency (NP) generated by the H command. The noise frequency defaults to 16.

|Values|Description|
|--|--|
|&lt;frequency&gt;|Specifies the noise frequency in the range 0 to 31.|

**Example:**
```
I0H4 I8H4
```

### &lt;

Lowers the octave specified by the O command.

**Example:**
```
L4O5C<BA
```

### &gt;

Raises the octave specified by the O command.

**Example:**
```
L4AB>C
```

### ,

MML of each part can be concatenated with commas. Since PSG has 3 tone channels, up to 3 MMLs can be concatenated.

**Example:**
```
T120L4O4CEG,
T120L4O4EGB,
T120L4O4GB>D
```

### & [&lt;octave-settings&gt;]

By connecting A-G commands with &, you can perform slurs and ties.

|Values|Description|
|--|--|
|&lt;octave-settings&gt;|You can use the O command, &gt; and &lt; to set the octave. The octave value set here will continue even after the performance of slurs and ties.|

**Example:**
```
A2R2 A4&A4R2 A2&>A0R2 A2&<A0R2
```

### [ [&lt;loop-number&gt;] ... ]

Loop playback of MML in []. Loops can be nested up to 5 levels. Loop symbols after the 6th row are ignored.

|Values|Description|
|--|--|
|&lt;loop-number&gt;|Specify the loop count in the range from 0 to 255. However, if 0 is specified, it will be an infinite loop. If this value is omitted, the loop count will be 1.|

**Example:**
```
[3
  [2
    L4CDER4
  ]
  L4GEDCDEDR4
]
```

## Software envelope generator

In addition to PSG's built-in envelope generator, psg_mml also supports volume control using software envelopes.
This feature can only be enabled if the volume is controlled with the V command rather than the S command.
The shape of the envelope is set with 6 parameters (AHDSFR method).
This envelope volume control can be set independently for each channel.

### $E &lt;enabled&gt;

Sets ON/OFF of volume control by software envelope. This value defaults to OFF.

|Values|Description|
|--|--|
|&lt;enabled&gt;|Set in the range from 0 to 1. 0 corresponds to OFF and 1 to ON.|

### $A &lt;attack&gt;

Specifies the rise time of the sound (the arrival time from 0 to the volume set by the V command).

|Values|Description|
|--|--|
|&lt;attack&gt;|Specify in the range from 0 to 10000. The unit is ms. If 0 is specified, sound rise processing is not performed, and sound is output at the volume set with the V command.|

### $H &lt;hold&gt;

Sets the retention time of the volume level after attack.

|Values|Description|
|--|--|
|&lt;hold&gt;|Specify in the range from 0 to 10000. The unit is ms. If 0 is specified, Decay processing will start immediately.|

### $D &lt;decay&gt;

Sets the time it takes for the volume to reach the Sustain value after Hold.

|Values|Description|
|--|--|
|&lt;decay&gt;|Specify in the range from 0 to 10000. The unit is ms. If 0 is specified, the volume is immediately set to the Sustain value and fade processing is started.|

### $S &lt;sustain&gt;

Sets the target volume level for Decay processing.

|Values|Description|
|--|--|
|&lt;sustain&gt;|Specify in the range from 0 to 100. The unit is %. Corresponds to the percentage of the volume specified by the V command.|

### $F &lt;fade&gt;

Specifies the time from the Sustain value to 0 after the Decay process is complete.

|Values|Description|
|--|--|
|&lt;fade&gt;|Specify in the range from 0 to 10000. The unit is ms. However, if 0 is specified, the volume will keep the Sustain value.|

### $R &lt;release&gt;

Specifies the time to change the output volume from the current value to 0 after note-off.

|Values|Description|
|--|--|
|&lt;release&gt;|Specify in the range from 0 to 10000. The unit is ms. However, if 0 is specified, the volume is immediately set to 0.|

## Software LFO

psg_mml has a software LFO. By enabling this function, you can output a sound with vibrato.

### $M &lt;mode&gt;

Sets ON/OFF for the software LFO. This value defaults to OFF.


|Values|Description|
|--|--|
|&lt;mode&gt;|Sets the software LFO mode from 0 to 1. 0 corresponds to OFF and 1 to ON (modulated by triangular wave).|

### $J &lt;depth&gt;

Sets the modulation depth. Modulation depth defaults to 0.

|Values|Description|
|--|--|
|&lt;depth&gt;|Specifies the modulation depth in the range 0 to 360. If the modulation depth is n, the frequency of the sound varies from 2^(-n/360) times to 2^(n/360) times depending on the modulation function.|

### $L &lt;low-frequency&gt;

Sets the modulation frequency. The default modulation frequency is 40 ( = 4.0 Hz).

|Values|Description|
|--|--|
|&lt;low-frequency&gt;|Specifies the modulation frequency in the range 0 to 200. The unit is 0.1 Hz.|

### $T &lt;delay&gt; [&lt;dot&gt;]

Specifies the time from the start of sound output until the LFO operates. This value defaults to 0.

|Values|Description|
|--|--|
|&lt;delay&gt;|Specifies the delay time of the LFO in the range from 0 to 64. If delay is non-zero, it uses the same calculations as the L command to determine the delay time. For example, if the delay is set to 4, the LFO operation will start after the time of one quarter note has passed after the start of sounding. If delay is 0, the delay time will be 0.|
|&lt;dot&gt;|Specifies a dot. The dot effect is the same as for A-G.|


## Other commands

### $B &lt;bias&gt;

Bias the frequency of the output sound. The default value for bias is 0.

|Values|Description|
|--|--|
|&lt;bias&gt;|Specify the bias in the range (-2880) to 2880. If the bias value is n, the frequency of the output sound will be the value multiplied by 2^(n/360).|

### $P &lt;pitchbend-level&gt;

Smoothly increases or decreases the frequency of the output sound to the value specified by pitchbend-level until the output stops. This value defaults to 0.

|Values|Description|
|--|--|
|&lt;pitchbend-level&gt;|Specifies the pitch bend level in the range from (-2880) to 2880. If the specified value is n, the frequency of the output sound changes smoothly up to the final value multiplied by 2^(n/360).|

# License

MIT License.



