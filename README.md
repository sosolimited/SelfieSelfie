# FrameTime
That Android+KeyGrip thing. On iOS.

## Building the project:
Android NDK should be extracted alongside the folder where you checked out this repository:

```
parent/
	android-ndk-r10d/
	FrameTime/
```

Once you have cloned everything, you will need to build Cinder for Android:

```bash
cd lib/Cinder-Android/android/
export NDK_ROOT=../../../../android-ndk-r10d/ # Make this path absolute or it won't work.
sh cbuilder.sh -j 8 -p 19 -a armeabi-v7a -b Debug -v -r
```

The main android project is being developed in the `SelfieSelfie/` directory. You can open it in Android Studio or XCode. Other Cinder projects were created along the way to test ideas on iOS. The `ShapeTool` project outputs the xml file describing the profile of the swept geometry.

## Getting set up to build Android (on OSX):

I kept track of the major steps I needed to follow to get Cinder-Android up and running in [Hai's Cinder wiki](https://github.com/chaoticbob/Cinder/wiki/Detailed-Dependency-Installation-Process).
