# SelfieSelfie

An Android Experiment by Sosolimited.

## Project Structure:

The main android project is being developed in the `SelfieSelfie/` directory. You can open it in Android Studio or XCode. Other Cinder projects were created along the way to test ideas on iOS. The `ShapeTool` project outputs the xml file describing the profile of the swept geometry.

In addition to this repo, you will need to install the Android NDK. The Android NDK should be extracted alongside this directory:

```
parent-directory/
	android-ndk-r10d/
	SelfieSelfie/
```

## Building the project:

Once you have cloned and extracted everything, you will need to build Cinder for Android (which lives as a submodule of this repository):

```bash
# Make sure you have the Cinder submodule pulled down.
git submodule update --init --recursive
```

```bash
cd lib/Cinder-Android/android/
# NDK_ROOT must be an absolute path.
export NDK_ROOT=/Users/soso/Code/android-ndk-r10d/
rm -Rf build/
sh cbuilder.sh -j 8 -p 19 -a armeabi-v7a -b Debug -v -r
sh cbuilder.sh -j 8 -p 19 -a armeabi-v7a -b Release -v -r

```

## Getting set up to build Android (on OSX):

We kept track of the major steps we needed to follow to get Cinder-Android up and running in [Hai's Cinder wiki](https://github.com/chaoticbob/Cinder/wiki/Detailed-Dependency-Installation-Process).
