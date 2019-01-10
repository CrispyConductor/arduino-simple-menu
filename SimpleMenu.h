#ifndef SimpleMenu_h
#define SimpleMenu_h

#include <inttypes.h>
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>

class SimpleMenu {

	public:
		SimpleMenu(LiquidCrystal_I2C *lcd, uint8_t numButtons, uint8_t const * buttonPins, uint8_t lcdCols = 16, uint8_t lcdRows = 2);
		void init(bool lcd = true, bool pins = true);

		uint8_t showMenu(char const * const * entries, uint8_t numEntries, char const * title = NULL);
		uint8_t showMenu_P(PGM_P const * entries, uint8_t numEntries, char const * title = NULL);
		void showMessage(char const * str, signed long maxWaitTimeMS = -1, char const * btn = NULL);
		void showMessage_P(char const * str, signed long maxWaitTimeMS = -1, char const * btn = NULL);

		float askFloat(float defaultVal = 0, uint8_t numDigits = 4, uint8_t numDecimals = 2, bool allowNegative = true);
		long askLong(long defaultVal = 0, uint8_t numDigits = 3, bool allowNegative = true);

		void printButtonLabels(char const * const * labels);
		void printPartialButtonLabels(char const * const * labels, uint8_t numLabels);
		uint8_t partialButtonOffset(uint8_t numLabels);
		void printMenuPage(char const * const * entries, uint8_t numEntries, uint8_t selected);
		void printMenuPage_P(PGM_P const * entries, uint8_t numEntries, uint8_t selected);
		void printLcdRow(char const * str, uint8_t row = 0);
		void clearLcdRow(uint8_t row = 0);
		void clearRestOfLcdRow();
		// returns 0xFF on timeout
		uint8_t waitForButtonPress(bool waitForRelease = true, signed long maxWaitMS = -1);
		void waitForAllButtonRelease(signed long maxWaitMS = -1);

		LiquidCrystal_I2C *lcd;
		uint8_t numButtons;
		uint8_t const * buttonPins;
		uint8_t lcdCols;
		uint8_t lcdRows;

};

#endif


