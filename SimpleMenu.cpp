
#include <Arduino.h>
#include <string.h>
#include "SimpleMenu.h"

SimpleMenu::SimpleMenu(LiquidCrystal_I2C *lcd, uint8_t numButtons, uint8_t const * buttonPins, uint8_t lcdCols, uint8_t lcdRows) {
	this->lcd = lcd;
	this->numButtons = numButtons;
	this->buttonPins = buttonPins;
	this->lcdCols = lcdCols;
	this->lcdRows = lcdRows;
}

void SimpleMenu::init(bool lcd, bool pins) {
	if (lcd) {
		this->lcd->init();
		this->lcd->backlight();
		this->lcd->clear();
		this->lcd->print("MENU");
	}
	if (pins) {
		for (uint8_t i = 0; i < numButtons; ++i) {
			pinMode(this->buttonPins[i], INPUT_PULLUP);
		}
	}
}


uint8_t SimpleMenu::partialButtonOffset(uint8_t numLabels) {
	return (numButtons - numLabels) / 2;
}

void SimpleMenu::printPartialButtonLabels(char const * const * labels, uint8_t numLabels) {
	char const *fullLabels[16];
	uint8_t labelOffset = partialButtonOffset(numLabels);
	for (uint8_t i = 0; i < numButtons; ++i) {
		if (i >= labelOffset && i < labelOffset + numLabels) {
			fullLabels[i] = labels[i - labelOffset];
		} else {
			fullLabels[i] = NULL;
		}
	}
	printButtonLabels(fullLabels);
}

void SimpleMenu::printButtonLabels(char const * const * labels) {
	uint8_t curLabelWidth = this->lcdCols / this->numButtons; // space on lcd taken up by this label, including padding and spacing
	this->lcd->setCursor(0, this->lcdRows - 1);
	uint8_t curLabelNum = 0; // current label in labels array
	uint8_t labelMaxLen = (this->lcdCols - this->numButtons + 1) / this->numButtons;
	uint8_t curLabelLen = labels[0] ? strlen(labels[0]) : 0; // length of current label string
	if (curLabelLen > labelMaxLen) curLabelLen = labelMaxLen;
	uint8_t curLabelPos = 0; // Current index within curLabelWidth being printed
	uint8_t curLabelPadding = (curLabelWidth - curLabelLen) / 2;
	for (uint8_t col = 0; col < this->lcdCols; ++col) {
		char curChar = ' ';
		if (curLabelPos >= curLabelPadding && curLabelPos < curLabelPadding + curLabelLen) {
			uint8_t labelIdx = curLabelPos - curLabelPadding;
			curChar = labels[curLabelNum][labelIdx];
		}
		this->lcd->write(curChar);

		curLabelPos++;
		if (curLabelPos == curLabelWidth) {
			curLabelPos = 0;
			curLabelNum++;
			if (curLabelNum < this->numButtons) {
				curLabelWidth = (this->lcdCols - (col+1)) / (this->numButtons - curLabelNum);
				curLabelLen = strlen(labels[curLabelNum]);
				if (curLabelLen > labelMaxLen) curLabelLen = labelMaxLen;
				curLabelPadding = (curLabelWidth - curLabelLen) / 2;
			}
		}
	}
}

void SimpleMenu::printMenuPage(char const * const * entries, uint8_t numEntries, uint8_t selected) {
	uint8_t availableRows = this->lcdRows - 1;
	uint8_t selectedRowOffset = (availableRows - 1) / 2;
	for (uint8_t row = 0; row < availableRows; ++row) {
		this->lcd->setCursor(0, row);
		int8_t entryNum = row + selected - selectedRowOffset;
		if (entryNum >= 0 && entryNum < numEntries) {
			if (entryNum == selected) {
				this->lcd->write('>');
			} else {
				this->lcd->write(' ');
			}
			uint8_t entryLen = strlen(entries[entryNum]);
			for (uint8_t i = 0; i < entryLen && i < this->lcdCols - 1; ++i) {
				this->lcd->write(entries[entryNum][i]);
			}
			for (uint8_t i = 0; i < this->lcdCols - 1; ++i) {
				this->lcd->write(' ');
			}
		} else {
			for (uint8_t i = 0; i < this->lcdCols; ++i) {
				this->lcd->write(' ');
			}
		}
	}
}

void SimpleMenu::printMenuPage_P(PGM_P const * entries, uint8_t numEntries, uint8_t selected) {
	uint8_t availableRows = this->lcdRows - 1;
	uint8_t selectedRowOffset = (availableRows - 1) / 2;
	char entryBuffer[33];
	for (uint8_t row = 0; row < availableRows; ++row) {
		this->lcd->setCursor(0, row);
		int8_t entryNum = row + selected - selectedRowOffset;
		if (entryNum >= 0 && entryNum < numEntries) {
			if (entryNum == selected) {
				this->lcd->write('>');
			} else {
				this->lcd->write(' ');
			}

			strcpy_P(entryBuffer, (PGM_P)pgm_read_word(&(entries[entryNum])));
			uint8_t entryLen = strlen(entryBuffer);
			for (uint8_t i = 0; i < entryLen && i < this->lcdCols - 1; ++i) {
				this->lcd->write(entryBuffer[i]);
			}
			for (uint8_t i = 0; i < this->lcdCols - 1; ++i) {
				this->lcd->write(' ');
			}
		} else {
			for (uint8_t i = 0; i < this->lcdCols; ++i) {
				this->lcd->write(' ');
			}
		}
	}
}

void SimpleMenu::waitForAllButtonRelease(signed long maxWait) {
	unsigned long endMillis = millis() + maxWait;
	// Wait for all button release
	bool allReleased = false;
	bool anyPressed = false;
	while(!allReleased) {
		if (maxWait >= 0 && millis() >= endMillis) return;
		allReleased = true;
		for (uint8_t i = 0; i < this->numButtons; ++i) {
			uint8_t value = digitalRead(this->buttonPins[i]);
			if (value == LOW) {
				allReleased = false;
			}
		}
		if (!allReleased) anyPressed = true;
	}
	if (anyPressed) delay(100); // debounce
}


uint8_t SimpleMenu::waitForButtonPress(bool waitForRelease, signed long maxWait) {
	unsigned long endMillis = millis() + maxWait;
	// Wait for all button release
	if (waitForRelease) {
		bool allReleased = false;
		while(!allReleased) {
			if (maxWait >= 0 && millis() >= endMillis) return 0xFF;
			allReleased = true;
			for (uint8_t i = 0; i < this->numButtons; ++i) {
				uint8_t value = digitalRead(this->buttonPins[i]);
				if (value == LOW) {
					allReleased = false;
				}
			}
		}
		delay(100); // debounce
	}
	// Wait for button press
	for (;;) {
		for (uint8_t i = 0; i < this->numButtons; ++i) {
			uint8_t value = digitalRead(this->buttonPins[i]);
			if (value == LOW) {
				delay(100); // debounce
				return i;
			}
		}
		if (maxWait >= 0 && millis() >= endMillis) return 0xFF;
	}
}

uint8_t SimpleMenu::showMenu(char const * const * entries, uint8_t numEntries, char const * title) {
	// Determine which button to use for UP/DOWN/SEL
	uint8_t btnUp = (this->numButtons - 3) / 2;
	uint8_t btnDown = btnUp + 1;
	uint8_t btnSel = btnDown + 1;

	char const *buttonLabels[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	buttonLabels[btnUp] = "UP";
	buttonLabels[btnDown] = "DOWN";
	buttonLabels[btnSel] = "SEL";

	if (title) {
		lcd->clear();
		printLcdRow(title, 0);
		printButtonLabels(buttonLabels);
		waitForButtonPress(true, 750);
	}

	uint8_t selected = 0;
	for (;;) {
		printMenuPage(entries, numEntries, selected);
		printButtonLabels(buttonLabels);
		int8_t btn = (uint8_t)waitForButtonPress();
		if (btn == btnUp) {
			selected--;
			if (selected < 0 || selected == 0xFF) selected = numEntries - 1;
		} else if (btn == btnDown) {
			selected++;
			if (selected >= numEntries) selected = 0;
		} else if (btn == btnSel) {
			return selected;
		}
	}
}

uint8_t SimpleMenu::showMenu_P(PGM_P const * entries, uint8_t numEntries, char const * title) {
	// Determine which button to use for UP/DOWN/SEL
	uint8_t btnUp = (this->numButtons - 3) / 2;
	uint8_t btnDown = btnUp + 1;
	uint8_t btnSel = btnDown + 1;

	char const *buttonLabels[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	buttonLabels[btnUp] = "UP";
	buttonLabels[btnDown] = "DOWN";
	buttonLabels[btnSel] = "SEL";

	if (title) {
		lcd->clear();
		printLcdRow(title, 0);
		printButtonLabels(buttonLabels);
		waitForButtonPress(true, 750);
	}

	uint8_t selected = 0;
	for (;;) {
		printMenuPage_P(entries, numEntries, selected);
		printButtonLabels(buttonLabels);
		int8_t btn = (uint8_t)waitForButtonPress();
		if (btn == btnUp) {
			selected--;
			if (selected < 0 || selected == 0xFF) selected = numEntries - 1;
		} else if (btn == btnDown) {
			selected++;
			if (selected >= numEntries) selected = 0;
		} else if (btn == btnSel) {
			return selected;
		}
	}
}

void SimpleMenu::printLcdRow(char const * str, uint8_t row) {
	this->lcd->setCursor(0, row);
	bool endOfStr = false;
	for (uint8_t i = 0; i < lcdCols; ++i) {
		char c;
		if (endOfStr) {
			c = ' ';
		} else if (str[i] == 0) {
			c = ' ';
			endOfStr = true;
		} else {
			c = str[i];
		}
		this->lcd->write(c);
	}
}

void SimpleMenu::clearLcdRow(uint8_t row) {
	printLcdRow("", row);
	lcd->setCursor(0, row);
}

void SimpleMenu::clearRestOfLcdRow() {
	for (uint8_t i = 0; i < lcdCols; ++i) lcd->write(' ');
}


void SimpleMenu::showMessage_P(char const * str, signed long timeout, char const * btn) {
	char buffer[20];
	strcpy_P(buffer, str);
	showMessage(buffer, timeout, btn);
}

void SimpleMenu::showMessage(char const * str, signed long timeout, char const * btn) {
	lcd->clear();
	char const * buttonLabels[] = { btn ? btn : "OK" };
	printPartialButtonLabels(buttonLabels, 1);
	lcd->setCursor(0, 0);
	lcd->print(str);
	waitForButtonPress(true, timeout);
}

uint8_t fixedSpaceLongStrLen(uint8_t numDigits, bool allowNegative) {
	return numDigits + (allowNegative ? 1 : 0);
}

void makeFixedSpaceLongStr(char *buf, long value, uint8_t numDigits, bool allowNegative) {
	uint8_t pos = 0;
	uint8_t totalLen = fixedSpaceLongStrLen(numDigits, allowNegative);
	long placeValue = 1;
	for (uint8_t i = 1; i < numDigits; ++i) placeValue *= 10L;

	if (allowNegative) {
		if (value < 0) {
			buf[pos++] = '-';
		} else {
			buf[pos++] = '+';
		}
	}

	if (value < 0) value = -value;

	for (; pos < totalLen; ++pos) {
		uint8_t digit = (uint8_t)(value / placeValue);
		value -= (long)digit * placeValue;
		placeValue /= 10L;
		buf[pos] = digit + '0';
	}
	buf[pos] = 0;
}

long parseFixedSpaceLongStr(char *str) {
	uint8_t pos = 0;
	long current = 0;
	bool negative = false;

	if (str[pos] == '-') {
		negative = true;
		++pos;
	} else if(str[pos] == '+') {
		++pos;
	}

	long placeValue = 1;
	for (uint8_t i = pos + 1; str[i]; ++i) {
		placeValue *= 10L;
	}

	for(; str[pos]; ++pos) {
		uint8_t digit = str[pos] - '0';
		current += placeValue * (long)digit;
		placeValue /= 10L;
	}

	if (negative) current = 0L - current;
	return current;
}

uint8_t fixedSpaceFloatStrLen(uint8_t numDigits, uint8_t numDecimals, bool allowNegative) {
	return numDigits + numDecimals + 1 + (allowNegative ? 1 : 0);
}

void makeFixedSpaceFloatStr(char *buf, float value, uint8_t numDigits, uint8_t numDecimals, bool allowNegative) {
	uint8_t pos = 0;
	uint8_t totalLen = fixedSpaceFloatStrLen(numDigits, numDecimals, allowNegative);
	float placeValue = 0.1;
	for (uint8_t i = 0; i < numDigits; ++i) placeValue *= 10.0f;

	if (allowNegative) {
		if (value < 0) {
			buf[pos++] = '-';
		} else {
			buf[pos++] = '+';
		}
	}

	if (value < 0) value = -value;

	for (; pos < totalLen; ++pos) {
		if (pos - (allowNegative ? 1 : 0) == numDigits) {
			buf[pos] = '.';
		} else {
			uint8_t digit = (uint8_t)(value / placeValue);
			value -= digit * placeValue;
			placeValue /= 10;
			buf[pos] = digit + '0';
		}
	}
	buf[pos] = 0;
}

float parseFixedSpaceFloatStr(char *str) {
	uint8_t pos = 0;
	float current = 0;
	bool negative = false;

	if (str[pos] == '-') {
		negative = true;
		++pos;
	} else if(str[pos] == '+') {
		++pos;
	}

	float placeValue = 0.1;
	for (uint8_t i = pos; str[i]; ++i) {
		if (str[i] == '.') break;
		placeValue *= 10.0f;
	}

	for(; str[pos]; ++pos) {
		if (str[pos] == '.') continue;
		uint8_t digit = str[pos] - '0';
		current += placeValue * (float)digit;
		placeValue /= 10.0f;
	}

	if (negative) current = 0 - current;
	return current;
}

long SimpleMenu::askLong(long defaultVal, uint8_t numDigits, bool allowNegative) {
	char const * buttonLabels[] = { "+", "-", "NEXT" };
	const char * endButtonLabels[] = { "+", "-", "DONE" };
	lcd->clear();
	printPartialButtonLabels(buttonLabels, 3);
	uint8_t buttonNumOffset = partialButtonOffset(3);

	uint8_t lcdNumberWidth = fixedSpaceLongStrLen(numDigits, allowNegative);
	char currentStr[17];
	makeFixedSpaceLongStr(currentStr, defaultVal, numDigits, allowNegative);
	uint8_t currentStrIdx = 0;
	uint8_t lcdColOffset = (lcdCols - lcdNumberWidth) / 2;

	lcd->setCursor(0, 0);
	for (uint8_t i = 0; i < lcdColOffset; ++i) lcd->write(' ');
	for (uint8_t i = 0; i < lcdNumberWidth; ++i) lcd->write(currentStr[i]);
	clearRestOfLcdRow();

	bool blink = true;
	for (;;) {
		if (currentStrIdx >= lcdNumberWidth) break;
		char c = currentStr[currentStrIdx];
		lcd->setCursor(lcdColOffset + currentStrIdx, 0);
		if (blink) {
			lcd->write(c);
		} else {
			lcd->write(' ');
		}
		blink = !blink;

		int8_t digit;
		bool isSignDigit = true;
		if (c == '+') {
			digit = 1;
		} else if (c == '-') {
			digit = -1;
		} else {
			isSignDigit = false;
			digit = c - '0';
		}

		uint8_t btn = waitForButtonPress(true, 750);

		if (btn == buttonNumOffset) {
			// + button
			if (isSignDigit) {
				currentStr[currentStrIdx] = '+';
				blink = true;
			} else {
				digit++;
				if (digit > 9) digit = 0;
				currentStr[currentStrIdx] = '0' + digit;
			}
		} else if (btn == buttonNumOffset + 1) {
			// - button
			if (isSignDigit) {
				currentStr[currentStrIdx] = '-';
				blink = true;
			} else {
				digit--;
				if (digit < 0) digit = 9;
				currentStr[currentStrIdx] = '0' + digit;
			}
		} else if (btn == buttonNumOffset + 2) {
			// Next button
			lcd->setCursor(lcdColOffset + currentStrIdx, 0);
			lcd->write(c);
			blink = true;
			currentStrIdx++;
			if (currentStrIdx >= lcdNumberWidth - 1) {
				printPartialButtonLabels(endButtonLabels, 3);
			}
		}
	}

	return parseFixedSpaceLongStr(currentStr);

}

float SimpleMenu::askFloat(float defaultVal, uint8_t numDigits, uint8_t numDecimals, bool allowNegative) {
	char const * buttonLabels[] = { "+", "-", "NEXT" };
	const char * endButtonLabels[] = { "+", "-", "DONE" };
	lcd->clear();
	printPartialButtonLabels(buttonLabels, 3);
	uint8_t buttonNumOffset = partialButtonOffset(3);

	uint8_t lcdNumberWidth = fixedSpaceFloatStrLen(numDigits, numDecimals, allowNegative);
	char currentStr[17];
	makeFixedSpaceFloatStr(currentStr, defaultVal, numDigits, numDecimals, allowNegative);
	uint8_t currentStrIdx = 0;
	uint8_t lcdColOffset = (lcdCols - lcdNumberWidth) / 2;

	lcd->setCursor(0, 0);
	for (uint8_t i = 0; i < lcdColOffset; ++i) lcd->write(' ');
	for (uint8_t i = 0; i < lcdNumberWidth; ++i) lcd->write(currentStr[i]);
	clearRestOfLcdRow();

	bool blink = true;
	for (;;) {
		if (currentStrIdx >= lcdNumberWidth) break;
		char c = currentStr[currentStrIdx];
		if (c == '.') {
			currentStrIdx++;
			continue;
		}

		lcd->setCursor(lcdColOffset + currentStrIdx, 0);
		if (blink) {
			lcd->write(c);
		} else {
			lcd->write(' ');
		}
		blink = !blink;

		int8_t digit;
		bool isSignDigit = true;
		if (c == '+') {
			digit = 1;
		} else if (c == '-') {
			digit = -1;
		} else {
			isSignDigit = false;
			digit = c - '0';
		}

		uint8_t btn = waitForButtonPress(true, 750);

		if (btn == buttonNumOffset) {
			// + button
			if (isSignDigit) {
				currentStr[currentStrIdx] = '+';
				blink = true;
			} else {
				digit++;
				if (digit > 9) digit = 0;
				currentStr[currentStrIdx] = '0' + digit;
			}
		} else if (btn == buttonNumOffset + 1) {
			// - button
			if (isSignDigit) {
				currentStr[currentStrIdx] = '-';
				blink = true;
			} else {
				digit--;
				if (digit < 0) digit = 9;
				currentStr[currentStrIdx] = '0' + digit;
			}
		} else if (btn == buttonNumOffset + 2) {
			// Next button
			lcd->setCursor(lcdColOffset + currentStrIdx, 0);
			lcd->write(c);
			blink = true;
			currentStrIdx++;
			if (currentStrIdx >= lcdNumberWidth - 1) {
				printPartialButtonLabels(endButtonLabels, 3);
			}
		}
	}

	return parseFixedSpaceFloatStr(currentStr);
}


