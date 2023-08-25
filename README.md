# positioning-system-stm32-dwm3000
This repo is for 2D and 3D positioning system based on DWM3000 and various version of STM32 Nucleo: G431KB, L432KC, F303K8. 
App which shows real time visualization of positioning system was made using MATLAB ver. 2022a.
**This is prototype, not for profesional use.**

# Important informations
This project was created as part of my Master's Thesis at the Silesian University of Technology.
System works on raw data, I do not use filters or algorithms to stabilize and improve accuracy of position.
In the system we can use 4 reference points or 6 reference points.

# Getting started
## Tasks in the system for STM32
In the system are 3 main tasks:
* Initiator
* Responder
* get message

During project I had 4x L432KC, 3x F303K8 and 3x G431KB <br>
Three L432KC do task Initiator (modules located on the object which is positioning) <br>
All three F303K8, two G431KB and one L432KC do task Responder (reference points).
One G431KB do task get message (module connected to the PC).

### Methodology of addresing modules
Responder: 1A, 2B, 3C, 4D, 5E, 6F <br>
Initiator: EV, FV, GV <br>
get message: CP <br>

## Import projects
At first you must create STM32CubeIDE Workspace. Then in the workspace folder clone a repository.

![obraz](https://github.com/PianistaPiano/positioning-system-stm32-dwm3000/assets/76052736/f67bcc53-3d34-40f1-9a4f-2257f0c28bd5)

In next step import projects to the STM32CubeIDE. 

![obraz](https://github.com/PianistaPiano/positioning-system-stm32-dwm3000/assets/76052736/e2ac5558-aadb-4d6a-98fd-1c9617bb3adc)


System is based on simple example of DS_TWR from Qorvo (ds_twr_initiator_sts.c and ds_twr_responder_sts.c). 

**DWM_Examples** - this project is configured for STM32 G431KB <br>
**DWM_Examples_F303K8** - this project is configured for STM32 F303K8 <br>
**DWM_Examples_L432KC** - this project is configured for STM32 L432KC <br>
**DWM_Examples_getMsg** - this project is configured for STM32 G431KB <br>

# Load and configure program to the STM32

For example if you want load task Initator on the L432KC with addres GV do this:

1. Open project DWM_Examples_L432KC
2. Go to Core->Inc->example_selection.h
3. Choose define TEST_DS_TWR_INITIATOR_STS and change INITIATOR_TYPE to 'G' <br>
![obraz](https://github.com/PianistaPiano/positioning-system-stm32-dwm3000/assets/76052736/2b872fc0-68f8-4274-957f-94a641c1b8a5) <br>
4. Open examples->ex_05a_ds_twr_init->ds_twr_initiator_sts.c
5. In define NUMBER_OF_ANCHORS choose 6 or 4 anchors.
6. Load program to the L432KC <br>

For example if you want load task Responder on the G431KB with addres 3C do this:

1. Open project DWM_Examples
2. Go to Core->Inc->example_selection.h
3. Choose define TEST_DS_TWR_RESPONDER_STS and change ANCHOR_TYPE to 'C' <br>
![obraz](https://github.com/PianistaPiano/positioning-system-stm32-dwm3000/assets/76052736/c13c37a4-c224-4a4c-acea-22526ee13032) <br>
4. Open examples->ex_05b_ds_twr_resp->ds_twr_responder_sts.c
5. Load program to the G431KB

To load task get message:

1. Open project DWM_Examples_getMsg
2. Go to examples->ex_get_msg_form_anchors->ex_get_msg_form_anchors.c
3. In define NUMBER_OF_ANCHORS choose 6 or 4 anchors.
4. Load program to the G431KB

# MATLAB App
To open app corectly you need MATLAB 2022a or higher.
## How to use?
1. Choose 2D or 3D positioning.
2. Enter number of anchors (4 or 6)
3. Enter positions of anchors
4. Enter room size
5. Choose serial port where "get message" device is connected
6. You can save entered informations to txt file (yellow button)
7. You can load file with settings (txt file must be in the same place where app files)
8. If everything is connected and turn on you can start positioning <br>

![obraz](https://github.com/PianistaPiano/positioning-system-stm32-dwm3000/assets/76052736/429978b0-4923-43ae-b76e-f8f668ffbc81)


# WARNING
If you want improve accuracy you must calibrate antena delay and PG Delay on every DWM3000 module.


