#include <FastLED.h>

#define PORT_COUNT 6
#define PORT_MAX_LIGHT_COUNT 12

#define PORT_1_PIN 3
#define PORT_1_LIGHT_COUNT 12

#define PORT_2_PIN 5
#define PORT_2_LIGHT_COUNT 12

#define PORT_3_PIN 6
#define PORT_3_LIGHT_COUNT 12

#define PORT_4_PIN 9
#define PORT_4_LIGHT_COUNT 12

#define PORT_5_PIN 10
#define PORT_5_LIGHT_COUNT 12

#define PORT_6_PIN 11
#define PORT_6_LIGHT_COUNT 12

CRGB leds[PORT_COUNT][PORT_MAX_LIGHT_COUNT];

const byte inputBufferSize = 32;
char input[inputBufferSize];
bool inputReady = false;

void setColor(int portIndex, int lightIndex, int r, int g, int b) {
	leds[portIndex][lightIndex].r = r;
	leds[portIndex][lightIndex].g = g;
	leds[portIndex][lightIndex].b = b;

	FastLED.show();
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

bool validateSerialInput(long port, long lightIndex, long r, long g, long b) {
	if (port <= 0 || port > PORT_COUNT) {
		return false;
	}

	if (lightIndex <= 0 || lightIndex > PORT_MAX_LIGHT_COUNT) { //TODO better validation
		return false;
	}

	if (r < 0 || r > 255 ||
		g < 0 || g > 255 ||
		b < 0 || b > 255) {
		return false;
	}

	return true;
}

void handleSerialInput() {
	if (!inputReady) {
		return;
	}

	char *pos;

	long portIndex = strtol(input, &pos, 10) - 1;
	long lightIndex = strtol(pos, &pos, 10) - 1;
	long r = strtol(pos, &pos, 16);
	long g = strtol(pos, &pos, 16);
	long b = strtol(pos, &pos, 16);

	if (validateSerialInput(portIndex, lightIndex, r, g, b)) {
		setColor(portIndex, lightIndex, r, g, b);
		Serial.println("ok");
	} else {
		Serial.println("bad");
	}

	inputReady = false;
}

void setup() {
	Serial.begin(115200);

	FastLED.addLeds<WS2812B, PORT_1_PIN, GRB>(leds[0], PORT_1_LIGHT_COUNT);
	FastLED.addLeds<WS2812B, PORT_2_PIN, GRB>(leds[0], PORT_2_LIGHT_COUNT);
	FastLED.addLeds<WS2812B, PORT_3_PIN, GRB>(leds[0], PORT_3_LIGHT_COUNT);
	FastLED.addLeds<WS2812B, PORT_4_PIN, GRB>(leds[0], PORT_4_LIGHT_COUNT);
	FastLED.addLeds<WS2812B, PORT_5_PIN, GRB>(leds[0], PORT_5_LIGHT_COUNT);
	FastLED.addLeds<WS2812B, PORT_6_PIN, GRB>(leds[0], PORT_6_LIGHT_COUNT);

	//FastLED.setCorrection(CRGB(255, 200, 200));

	//TODO initializeState();

	Serial.println("argb-controller v0.1");
	Serial.println("ready");
}

void loop() {
	readSerialInput();
	handleSerialInput();
}