#include <EEPROM.h>
#include <TimerOne.h>
#include "ClickEncoder.h"
#include "HSVtoRGB.h"

#define PIN_RE_A 2
#define PIN_RE_B 3
#define PIN_RE_BTN A3
#define PIN_LED_R 9
#define PIN_LED_G 10
#define PIN_LED_B 11

ClickEncoder *encoder;

struct Color { int h; double s; double l; } color { 0, 1, 1 };

enum Mode {
	Hue,
	Saturation,
	Level,
	Off
} mode = Hue;

long buttonHoldStart = 0;

void loadStoredColor() {
	color.h = map(EEPROM.read(0), 0, 255, 0, 360);
	color.s = EEPROM.read(1) / 100;
	color.l = EEPROM.read(2) / 100;
}

void storeColor() {
	EEPROM.write(0, map(color.h, 0, 360, 0, 255));
	EEPROM.write(1, color.s * 100);
	EEPROM.write(2, color.l * 100);
}

void updateColor() {
	int rgb[3];
	HSVtoRGB(color.h, color.s, color.l, rgb);

	analogWrite(PIN_LED_R, rgb[0]);
	analogWrite(PIN_LED_G, rgb[1]);
	analogWrite(PIN_LED_B, rgb[2]);
}

void adjustHue(int change) {
	int hue = color.h + change;

	if (hue > 360) {
		hue = color.h - 360;
	} else if (hue < 0) {
		hue = 0 - color.h;
	}

	color.h = hue;
	updateColor();
}

void adjustSaturation(int change) {
	double saturation = color.s + (change * 0.01);

	if (saturation > 1) {
		saturation = 1;
	} else if (saturation < 0) {
		saturation = 0;
	}

	color.s = saturation;
	updateColor();
}

void adjustLevel(int change) {
	double level = color.l + (change * 0.01);

	if (level > 1) {
		level = 1;
	} else if (level < 0) {
		level = 0;
	}

	color.l = level;
	updateColor();
}

void blink(int count) {
	int originalLevel = color.l;
	for (int i = 0; i < count; i++) {
		if (i) { delay(150); }
		color.l = 0;
		updateColor();

		delay(150);
		color.l = originalLevel;
		updateColor();
	}
}

void togglePower() {
	if (mode == Off) {
		color.l = 1;
		mode = Hue;
	} else {
		color.l = 0;
		mode = Off;
	}

	updateColor();
}

void toggleMode() {
	switch (mode) {
		case Hue: mode = Saturation; blink(2); break;
		case Saturation: mode = Level; blink(3); break;
		case Level: mode = Hue; blink(1); break;
	}
}

void setStartupColor() {
	storeColor();
	blink(3);
}

void setup() {
	Serial.begin(115200);

	pinMode(PIN_LED_R, OUTPUT);
	pinMode(PIN_LED_G, OUTPUT);
	pinMode(PIN_LED_B, OUTPUT);

	loadStoredColor();
	updateColor();

	encoder = new ClickEncoder(PIN_RE_A, PIN_RE_B, PIN_RE_BTN, 4);

	Timer1.initialize(1000);
	Timer1.attachInterrupt([]() { encoder->service(); });

	Serial.println("Setup complete.");
}

void loop() {
	int change = encoder->getValue();
	if (mode != Off && change) {
		switch (mode) {
			case Hue: adjustHue(change * 2); break;
			case Saturation: adjustSaturation(change * 2); break;
			case Level: adjustLevel(change * 2); break;
		}
	}

	ClickEncoder::Button button = encoder->getButton();
	switch (button) {
		case ClickEncoder::Clicked: toggleMode(); break;
		case ClickEncoder::DoubleClicked: togglePower(); break;
		case ClickEncoder::Held:
			if (buttonHoldStart == 0) {
				buttonHoldStart = millis();
			} else if (millis() - buttonHoldStart >= 1500) {
				buttonHoldStart = -1;
				setStartupColor();
			}
			break;
		case ClickEncoder::Released:
			buttonHoldStart = 0;
			break;
	}
}