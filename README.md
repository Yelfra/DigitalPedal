# DigitalPedal
This was part of a two-part project including implementations of a guitar delay pedal using solely software and solely hardware.

## Description
DigitalPedal is the software implementation and imitation of its hardware counterpart of the delay pedal for electric guitar. It allows applying the delay effect to microphone input and playing the result simultaneously.
Controlling the digital pedal is performed via the keyboard in real-time. More precisely, toggling the delay effect off/on and adjusting the delay time and feedback factor.
The application interface consists of a window with a black background with user instructions and control feedback written in white text.

## Technical
Software implementation was achieved using the C++ programming language in the Visual Studio environment for Windows 10, with great help from the PortAudio library.
By using the library's API, greater focus could be placed on the implementation of the delay effect, while also preventing unnecessary errors and complications of interacting with hardware and lower levels of the operating system.

PortAudio's major role here is the stream of audio data in 32-bit floating point format from input to output device. It achieves this through input and output buffers, which the application along with its delay buffer then manipulates through iterative cycles.
With each iteration, data from the delay buffer is read to the output buffer, while data from the input buffer is read to the delay buffer. This way data from the current iteration is read in the next iteration, simulating a delay. The delay effect is basically achieved through the size of the delay buffer, which is defined by the product of delay time and sample rate. The user's adjustment of delay time proportionally adjusts this size.

To truly achieve the delay effect, in addition to ordinary delay, feedback is added. This implementation is trivial and is based on adding the product of the feedback factor and the delayed sample. The feedback factor can be adjusted by the user within 0 and 1. A feedback factor of 0 will turn off the echoing effect, while a value of 1 will simulate an infinite echo without damping.

## How to Use
1. Run DigitalPedal.exe.
  - Recording and playback will immediately start.
  - IMPORTANT NOTE: For the application to run correctly, it is essential to have the portaudio_x64.dll file in the same directory.
3. Toggle the delay effect on/off using SPACEBAR.
4. Adjust delay time using LEFT/RIGHT ARROW KEYS.
5. Adjust feedback factor using UP/DOWN ARROW KEYS.
6. Exit the application by pressing ENTER or ESCAPE.

## PortAudio
PortAudio library is a free, cross-platform, open-source library for easier transfer and processing of audio data from input to output devices.
https://www.portaudio.com
