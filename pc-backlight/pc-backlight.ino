#include <FastLED.h>
#include "Config.h"

struct {
	int brightness;
	int rgb[3];
} state;

bool isPowerOn = false;
CRGB leds[LIGHT_COUNT];

const byte inputBufferSize = 32;
char input[inputBufferSize];
bool inputReady = false;

void initializeState() {
	state.rgb[0] = 255;
	state.rgb[1] = 255;
	state.rgb[2] = 255;
	state.brightness = 100;
}

void setColor(int r, int g, int b) {
	state.rgb[0] = r;
	state.rgb[1] = g;
	state.rgb[2] = b;

	fill_solid(leds, LIGHT_COUNT, CRGB(r, g, b));
	FastLED.show();
}

void fadeToBrightness(int level) {
	while (state.brightness != level) {
		if (state.brightness < level) {
			state.brightness++;
		} else {
			state.brightness--;
		}
		
		FastLED.setBrightness(map(state.brightness, 0, 100, 0, 255));
		FastLED.show();

		if (state.brightness != level) {
			FastLED.delay(10);
		}
	}
}

void updateState(int r, int g, int b, int level) {
	//if (r != state.rgb[0] || g != state.rgb[1] || b != state.rgb[2]) {
		//fadeToBrightness(0);
		setColor(r, g, b);
	//}

	fadeToBrightness(level);
}

void readSerialInput() {
	static byte index = 0;
	char current;

	while (Serial.available() > 0 && !inputReady) {
		current = Serial.read();

		if (current != '\n') {
			input[index++] = current;

			if (index >= inputBufferSize) {
				index = inputBufferSize - 1;
			}

			continue;
		}
		
		input[index] = '\0';
		index = 0;

		Serial.println("thanks");

		inputReady = true;
	}
}

void handleSerialInput() {
	if (!inputReady) {
		return;
	}

	const byte bufferSize = 5;
	const byte valuesSize = 4;

	char buffer[bufferSize];
	int values[valuesSize];

	int bufferIndex = 0;
	int valuesIndex = 0;

	for (int i = 0; i < inputBufferSize; i++) {
		bool isTerminated = input[i] == '\0';

		if (bufferIndex >= bufferSize) {
			bufferIndex = bufferSize - 1;
		}

		if (input[i] == ',' || isTerminated) {
			buffer[bufferIndex] = '\0';
			bufferIndex = 0;

			if (valuesIndex < valuesSize) {
				values[valuesIndex++] = atoi(buffer);
			}
		} else {
			buffer[bufferIndex++] = input[i];
		}

		if (isTerminated) {
			break;
		}
	}

	if (valuesIndex == valuesSize && 
		values[0] >= 0 && values[0] <= 255 &&
		values[1] >= 0 && values[1] <= 255 &&
		values[2] >= 0 && values[2] <= 255 &&
		values[3] >= 0 && values[3] <= 100) {
		updateState(values[0], values[1], values[2], values[3]);
		Serial.println("ok");
	} else {
		Serial.println("bad");
	}

	inputReady = false;
}

void setup() {
	Serial.begin(115200);

	FastLED.addLeds<WS2812B, LIGHT_PIN, GRB>(leds, LIGHT_COUNT);
	FastLED.setCorrection(CRGB(255, 200, 200));

	initializeState();

	Serial.println("pc-backlight");
	Serial.println("ready");
}

void loop() {
	readSerialInput();
	handleSerialInput();
}