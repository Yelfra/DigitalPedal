#include <iostream>
#include <vector>
#include <cmath>
#include <conio.h>

#include "portaudio.h"

// Configuration constants
constexpr uint16_t SAMPLE_RATE = 44100; // Hz
constexpr size_t BUFFER_SIZE = 512; // bytes
constexpr float DELAY_TIME = 0.5;  // seconds

// Simple Delay class
class Delay {
private:
	size_t delayBufferSize;
	int readIndex;
	int writeIndex;
	std::vector<float> delayBuffer;
	uint16_t sampleRate;

public:
	Delay(float delayTime, int sampleRate) {
		this->delayTime = delayTime;
		this->sampleRate = sampleRate;
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

	bool isDelayActive = false;
	void toggleDelay() {
		isDelayActive = !isDelayActive;
	}

	float delayTime;
	void adjustDelayTime(float value) {
		delayTime += value;
		if(delayTime < 0.1) {
			delayTime = 0.1;
		}
		delayBufferSize = static_cast<size_t>(delayTime * sampleRate);
		delayBuffer.resize(delayBufferSize, 0.0f);
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
		float outputSample = delay->isDelayActive ? delay->process(inputSample) : inputSample;
		*out++ = outputSample;
		*out++ = outputSample;  // Stereo output (duplicate for simplicity)
	}

	return paContinue;
}

int main() {
	//// PORTAUDIO:
	PaStream* stream;
	PaError err;

	/// CONTROL:
	bool isArrowKeyPressed = false;
	float delayTimeIncrement = 0.05; // seconds

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

	std::cout << "----DELAY-----" << std::endl;
	std::cout << "Delay Time: " << delayEffect.delayTime << " s" << std::endl;

	std::cout << "---CONTROLS---" << std::endl;
	std::cout << "Toggle Delay: Spacebar" << std::endl;
	std::cout << "Adjust Delay Time: <- / ->" << std::endl;
	std::cout << "Exit: Enter / ESC" << std::endl << std::endl;

	while(true) {
		char key = _getch();

		//// DELAY TIME:
		if(key == -32 || isArrowKeyPressed) {
			isArrowKeyPressed = true;

			// Left arrow
			if(key == 75) {
				delayEffect.adjustDelayTime(-delayTimeIncrement);
				std::cout << "Delay Time: " << delayEffect.delayTime << " s" << std::endl;
				isArrowKeyPressed = false;
			}
			// Right arrow
			else if(key == 77) {
				delayEffect.adjustDelayTime(delayTimeIncrement);
				std::cout << "Delay Time: " << delayEffect.delayTime << " s" << std::endl;
				isArrowKeyPressed = false;
			}
		}
		//// TOGGLE DELAY:
		// Spacebar
		else if(key == 32) {
			delayEffect.toggleDelay();
			std::cout << "Delay Effect " << (delayEffect.isDelayActive ? "ON" : "OFF") << std::endl;
		}
		//// EXIT:
		// Enter || ESC
		else if(key == 13 || key == 27) {
			break;
		}
	}

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
