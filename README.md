# ORB-SLAM2 for Window 執行步驟 
## 將以下**ORBSLAM24Windows建置完成後** 開啟CMD

第一步：進到執行資料夾

```powershell
cd C:\Users\853uj\VSproject\ORB-SLAM2\ORB_SLAM2_for_Windows\Examples\Monocular\Release
```

第二步：輸入執行指令 ⇒ 執行檔 path_to_vocabulary(.txt) path_to_settings(.yaml) path_to_sequence(folder) 

```python
mono_tum.exe C:\Users\853uj\VSproject\ORB-SLAM2\ORB_SLAM2_for_Windows\Vocabulary\ORBvoc.txt C:\Users\853uj\VSproject\ORB-SLAM2\ORB_SLAM2_for_Windows\Examples\Monocular\TUM2.yaml C:\Users\853uj\Downloads\rgbd_dataset_freiburg2_desk
```

## **ORBSLAM24Windows**

ORBSLAM2 Project 4(for) Windows Platform

Easy built orbslam2 by visual studio on windows of both debug and release mode

### **Thanks**

- Original ORBSLAM2 project: [ORB_SLAM2](https://github.com/raulmur/ORB_SLAM2)
- Eigen and Pangolin in Thirdparty are extracted from [Phylliida/orbslam-windows/Thirdparty](https://github.com/Phylliida/orbslam-windows/tree/master/Thirdparty)

### **Prerequisite `預先裝好 OpenCV CMake VisualStudio2022(以下2013皆改為2022即可)`**

1. OpenCV ([安裝步驟](https://www.notion.so/Opencv-4-5-0-darknet-install-Windows-1ea2ac42b1da80b7b758d27b48d44e2e?pvs=21))
    - Version is not required, but not too old. In this tutorial is 2.4.13.
    - Add `YOUR_OWN_PATH\opencv\build;` `YOUR_OWN_PATH\opencv\build\x64\vc12\bin;` to your environment variable "PATH", you can also add `YOUR_OWN_PATH\opencv\build\x86\vc12\bin;` if you want to bulid a x86 type application.
2. Cmake GUI (裝好了就能用)
    - Version should at least be 2.8.
3. Visual Studio (裝好了就能用)
    - In this tutorial is VS2013(Corresponding to opencv's vc12).

So, we'll build a visual studio 2013 project of ORB_SLAM2 using cmake and then make a x64 app.

### **Steps `安裝與建置外部函式庫(使用CMake GUI)`**

First, we'll compile the projects in **Thirdparty** folder.

### **DBoW2 `跳什麼問題就改什麼(主要需要改CMAKE和OPENCV版本)`**

1. Open cmake-gui, select DBow2 folder as the source path and the DBow2/build folder as the binaries path.
2. Click configure, select Visual Studio 12 2013 Win64(or your own) as the generator, click finish.
3. After configure done, click Generate.
4. Go to the DBow2/build folder, double click the DBoW2.sln to open the peoject.
5. Build ALL_BUILD in either debug or release mode you want.
6. After success build, the libraries will be in the lib folder of the DBow2 project source folder.

### **eigen**

**eigen is not need to be built**

### **g2o**

1. Open cmake-gui, select g2o folder as the source path and the g2o/build folder as the binaries path.
2. Click configure, select Visual Studio 12 2013 Win64(or your own) as the generator, click finish.
3. After configure done, click Generate.
4. Go to the g2o/build folder, double click the g2o.sln to open the peoject.
5. Right click on the g2o project->Properties->C/C++->Preprocessor Definitions, add WINDOWS at the end row, click Apply and OK.
6. Build ALL_BUILD in either debug or release mode you want. **(Remind to repeat step 5 && Mode should be the same as DBoW2)**
7. After success build, the libraries will be in the lib folder of the g2o project source folder.

### **Pangolin**

1. Open cmake-gui, select Pangolin folder as the source path and the Pangolin/build folder as the binaries path.
2. Click configure, select Visual Studio 12 2013 Win64(or your own) as the generator, click finish.
3. After configure done, click Generate.
4. Go to the Pangolin/build folder, double click the Pangolin.sln to open the peoject.
5. Build ALL_BUILD in either debug or release mode you want. **(Mode should be the same as DBoW2 && g2o)**.
6. You'll get a error of "cannot open input file 'pthread.lib'", just ignore it.
7. After success build, the libraries will be in the lib folder of the Pangolin project source folder.

### **ORBSLAM24Windows ([OpenCV大改](https://www.notion.so/ORB_SLAM-opencv-1eb2ac42b1da80328166f969498aaec6?pvs=21))**

1. Open cmake-gui, select ORBSLAM24Windows folder as the source path and the ORBSLAM24Windows/build folder as the binaries path.
2. Click configure, select Visual Studio 12 2013 Win64(or your own) as the generator, click finish.
3. After configure done, click Generate.
4. Go to the ORBSLAM24Windows/build folder, double click the ORB_SLAM2.sln to open the peoject.
5. Choose either debug or release mode you want. **(Mode should be the same as DBoW2 && g2o && Pangolin)**.
6. Right click the ORB_SLAM2 project and then click generate.
7. After success build, the libraries will be in the lib folder of the ORB_SLAM2 project source folder.

# **TODO Applications `看能不能用在上次拍的數據上`**

If you want to make apps, you can also build the mono-stero-RGBD projects provided.

Take mono_tum app as an example, you can follow the steps below.

1. Go to the ORBSLAM24Windows/build folder, double click the ORB_SLAM2.sln to open the peoject.
2. Choose either debug or release mode you want. **(Build mode should be the same as DBoW2 && g2o && Pangolin && ORB_SLAM2)**.
3. Right click the mono_tum project and then click generate.
4. Download tum dataset sequence, for example [freiburg2_desk](https://cvg.cit.tum.de/rgbd/dataset/freiburg2/rgbd_dataset_freiburg2_desk.tgz) { [TUM download](https://cvg.cit.tum.de/data/datasets/rgbd-dataset/download) }
5. Right click the mono_tum project and then click Property->Config Property->Debug, input three parameters (Usage: ./mono_tum path_to_vocabulary path_to_settings path_to_sequence, the first can be ignored in windows) 
    - **path_to_vocabulary** In ORBSLAM24Windows/Vocabulary folder, unpack the tar, a .txt file
    - **path_to_settings** In ORBSLAM24Windows/Examples/Monocular folder, rgbd_dataset_freiburg2_desk corresponding to TUM2.yaml
    - **path_to_sequence** rgbd_dataset_freiburg2_desk folder path
6. Run app, it'll take a few minutes to load the vocabulary dictionary, and then you'll get the result.
7. `5. 6. 步可以改為使用：開啟CMD terminal執行`

第一步：進到執行資料夾

```powershell
cd C:\Users\853uj\VSproject\ORB-SLAM2\ORB_SLAM2_for_Windows\Examples\Monocular\Release
```

第二步：輸入執行指令 ⇒ 執行檔 path_to_vocabulary(.txt) path_to_settings(.yaml) path_to_sequence(folder) 

```python
mono_tum.exe C:\Users\853uj\VSproject\ORB-SLAM2\ORB_SLAM2_for_Windows\Vocabulary\ORBvoc.txt C:\Users\853uj\VSproject\ORB-SLAM2\ORB_SLAM2_for_Windows\Examples\Monocular\TUM2.yaml C:\Users\853uj\Downloads\rgbd_dataset_freiburg2_desk
```

If you don't satisfied with the speed of loading dictionary, you can reference issue [vocabulary convert](https://github.com/raulmur/ORB_SLAM2/pull/21) to convert the txt vocabulary to bin vocabulary, it speeds up a lot.

The picture shows the result of mono TUM dataset.

![](https://github.com/phdsky/ORBSLAM24Windows/raw/master/ORBSLAM2_monoTUM.png)


## // Terminal 執行TUM dataset時 輸出內容

```bash
ORB-SLAM2 Copyright (C) 2014-2016 Raul Mur-Artal, University of Zaragoza.
This program comes with ABSOLUTELY NO WARRANTY;
This is free software, and you are welcome to redistribute it
under certain conditions. See LICENSE.txt.

Input sensor was set to: Monocular

Loading ORB Vocabulary. This could take a while...
Vocabulary loaded!

Camera Parameters: 
- fx: 520.909
- fy: 521.007
- cx: 325.141
- cy: 249.702
- k1: 0.231222
- k2: -0.784899
- k3: 0.917205
- p1: -0.003257
- p2: -0.000105
- fps: 30
- color order: RGB (ignored if grayscale)

ORB Extractor Parameters: 
- Number of Features: 1000
- Scale Levels: 8
- Scale Factor: 1.2
- Initial Fast Threshold: 20
- Minimum Fast Threshold: 7

-------
Start processing sequence ...
Images in the sequence: 2965

New Map created with 92 points
Wrong initialization, reseting...
System Reseting
Reseting Local Mapper... done
Reseting Loop Closing... done
Reseting Database... done
New Map created with 110 points
Loop detected!
Local Mapping STOP
Local Mapping RELEASE
Starting Global Bundle Adjustment
Global Bundle Adjustment finished
Updating map ...
Local Mapping STOP
Local Mapping RELEASE
Map updated!
-------

median tracking time: 0.0362737
mean tracking time: 0.0369288

Saving keyframe trajectory to KeyFrameTrajectory.txt ...

trajectory saved!
```
