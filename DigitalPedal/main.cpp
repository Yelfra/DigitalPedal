#include <iostream>
#include <vector>
#include <cmath>
#include <conio.h>

#include "portaudio.h"

// Configuration constants
constexpr uint16_t SAMPLE_RATE = 44100; // Hz
constexpr size_t BUFFER_SIZE = 512; // bytes
constexpr float DELAY_TIME = 0.5f;  // seconds
constexpr float DELAY_TIME_DELTA = 0.05f; // seconds
constexpr float FEEDBACK = 0.5f;  // from 0 to 1
constexpr float FEEDBACK_DELTA = 0.05f;

// Simple Delay class
class Delay {
private:
	uint16_t sampleRate;

	size_t delayBufferSize;
	int readIndex;
	int writeIndex;
	std::vector<float> delayBuffer;

public:
	Delay(float delayTime, int sampleRate, float feedback)
		: delayTime(delayTime), sampleRate(sampleRate), feedback(feedback) {

		delayBufferSize = static_cast<size_t>(delayTime * sampleRate);
		delayBuffer.resize(delayBufferSize, 0.0f);
		readIndex = 0;
		writeIndex = 0;
	}

	float process(float input) {
		float delayedSample = delayBuffer[readIndex];
		delayBuffer[writeIndex] = input + feedback * delayedSample;

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
		if(delayTime < DELAY_TIME_DELTA) {
			delayTime = DELAY_TIME_DELTA;
		}
		delayBufferSize = static_cast<size_t>(delayTime * sampleRate);
		delayBuffer.resize(delayBufferSize, 0.0f);
	}
	
	float feedback;
	void adjustFeedback(float value) {
		feedback += value;
		if(feedback < 0) {
			feedback = 0;
		} else if(feedback > 1) {
			feedback = 1;
		}
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

	// Initialize PortAudio
	err = Pa_Initialize();
	if(err != paNoError) {
		std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
		return -1;
	}

	// Create delay effect
	Delay delayEffect(DELAY_TIME, SAMPLE_RATE, FEEDBACK);

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
	std::cout << "Adjust Delay Time: LEFT / RIGHT arrow keys" << std::endl;
	std::cout << "Adjust Feedback: DOWN / UP arrow keys" << std::endl;
	std::cout << "Exit: Enter / ESC" << std::endl << std::endl;

	while(true) {
		char key = _getch();

		//// ADJUST:
		// Arrow
		if(key == -32 || isArrowKeyPressed) {
			isArrowKeyPressed = true;

			//// DELAY TIME:
			// Left arrow
			if(key == 75) {
				delayEffect.adjustDelayTime(-DELAY_TIME_DELTA);
				std::cout << "Delay Time: " << delayEffect.delayTime << " s" << std::endl;
				isArrowKeyPressed = false;
			}
			// Right arrow
			else if(key == 77) {
				delayEffect.adjustDelayTime(DELAY_TIME_DELTA);
				std::cout << "Delay Time: " << delayEffect.delayTime << " s" << std::endl;
				isArrowKeyPressed = false;
			}

			//// FEEDBACK:
			// Down arrow
			if(key == 80) {
				delayEffect.adjustFeedback(-FEEDBACK_DELTA);
				std::cout << "Feedback: " << delayEffect.feedback << std::endl;
				isArrowKeyPressed = false;
			}
			// Up arrow
			else if(key == 72) {
				delayEffect.adjustFeedback(FEEDBACK_DELTA);
				std::cout << "Feedback: " << delayEffect.feedback << std::endl;
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
