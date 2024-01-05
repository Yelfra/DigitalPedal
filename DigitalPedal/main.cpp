#include <iostream>
#include <vector>
#include <cmath>

#include "portaudio.h"

// Configuration constants
constexpr size_t SAMPLE_RATE = 44100; // Hz
constexpr size_t BUFFER_SIZE = 512; // bytes
constexpr float DELAY_TIME = 0.5;  // seconds

// Simple Delay class
class Delay {
private:
    size_t delayBufferSize;
    int readIndex;
    int writeIndex;
    std::vector<float> delayBuffer;

public:
    Delay(float delayTime, int sampleRate) {
        delayBufferSize = static_cast<size_t>(delayTime * sampleRate);
        delayBuffer.resize(delayBufferSize, 0.0f);
        readIndex = 0;
        writeIndex = 0;
    }

    float process(float input) {
        float delayedSample = delayBuffer[readIndex];
        delayBuffer[writeIndex] = input;

        readIndex = (readIndex + 1) % delayBufferSize;
        writeIndex = (writeIndex + 1) % delayBufferSize;

        return delayedSample;
    }
};

// PortAudio callback function
static int paCallback(const void* inputBuffer, void* outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void* userData) {
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;
    Delay* delay = (Delay*)userData;

    for(unsigned int i = 0; i < framesPerBuffer; i++) {
        float inputSample = *in++;
        float outputSample = delay->process(inputSample);
        *out++ = outputSample;
        *out++ = outputSample;  // Stereo output (duplicate for simplicity)
    }

    return paContinue;
}

int main() {
    PaStream* stream;
    PaError err;

    // Initialize PortAudio
    err = Pa_Initialize();
    if(err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    // Create delay effect
    Delay delayEffect(DELAY_TIME, SAMPLE_RATE);

    // Open PortAudio stream
    err = Pa_OpenDefaultStream(&stream, 1, 2, paFloat32, SAMPLE_RATE, BUFFER_SIZE, paCallback, &delayEffect);
    if(err != paNoError) {
        std::cerr << "PortAudio stream opening failed: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return -1;
    }

    // Start the stream
    err = Pa_StartStream(stream);
    if(err != paNoError) {
        std::cerr << "PortAudio stream starting failed: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return -1;
    }

    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();

    // Stop and close the stream
    err = Pa_StopStream(stream);
    if(err != paNoError) {
        std::cerr << "PortAudio stream stopping failed: " << Pa_GetErrorText(err) << std::endl;
    }
    err = Pa_CloseStream(stream);
    if(err != paNoError) {
        std::cerr << "PortAudio stream closing failed: " << Pa_GetErrorText(err) << std::endl;
    }

    // Terminate PortAudio
    Pa_Terminate();

    return 0;
}
