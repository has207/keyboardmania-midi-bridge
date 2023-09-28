#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "hidapi.h"
#include <rtmidi.h>
#include <map>

#define MAX_STR 255

typedef struct {
	unsigned char byteIndex;
	unsigned char mask;
	const char* name;
} KeyMapping;

KeyMapping KEY_MAPPINGS[] = {
	{1, 0x40, "Select"},
	{2, 0x40, "Start"},
	{3, 0x20, "Whl-Up"},
	{3, 0x40, "Whl-Dn"},
	{0, 0x01, "C3"},
	{0, 0x02, "C#3"},
	{0, 0x04, "D3"},
	{0, 0x08, "D#3"},
	{0, 0x10, "E3"},
	{0, 0x20, "F3"},
	{0, 0x40, "F#3"},
	{1, 0x01, "G3"},
	{1, 0x02, "G#3"},
	{1, 0x04, "A3"},
	{1, 0x08, "A#3"},
	{1, 0x10, "B3"},
	{1, 0x20, "C4"},
	{2, 0x01, "C#4"},
	{2, 0x02, "D4"},
	{2, 0x04, "D#4"},
	{2, 0x08, "E4"},
	{2, 0x10, "F4"},
	{2, 0x20, "F#4"},
	{3, 0x01, "G4"},
	{3, 0x02, "G#4"},
	{3, 0x04, "A4"},
	{3, 0x08, "A#4"},
	{3, 0x10, "B4"},
};

std::map<uint32_t, uint8_t> NOTE_MAPPINGS = {
	// Control buttons
	{0x00400000, 0},  // Select
	{0x00004000, 1},  // Start
	{0x00000020, 2},  // Whl-Up
	{0x00000040, 3},  // Whl-Dn

	// Note buttons
	{0x01000000, 48}, // C3
	{0x02000000, 49}, // C#3
	{0x04000000, 50}, // D3
	{0x08000000, 51}, // D#3
	{0x10000000, 52}, // E3
	{0x20000000, 53}, // F3
	{0x40000000, 54}, // F#3
	{0x00010000, 55}, // G3
	{0x00020000, 56}, // G#3
	{0x00040000, 57}, // A3
	{0x00080000, 58}, // A#3
	{0x00100000, 59}, // B3

	{0x00200000, 60}, // C4
	{0x00000100, 61}, // C#4
	{0x00000200, 62}, // D4
	{0x00000400, 63}, // D#4
	{0x00000800, 64}, // E4
	{0x00001000, 65}, // F4
	{0x00002000, 66}, // F#4
	{0x00000001, 67}, // G4
	{0x00000002, 68}, // G#4
	{0x00000004, 69}, // A4
	{0x00000008, 70}, // A#4
	{0x00000010, 71}  // B4
};


void printPressedKeys(const unsigned char* buffer) {
	printf("[ ");
	for (int i = 0; i < sizeof(KEY_MAPPINGS) / sizeof(KeyMapping); i++) {
		if (buffer[KEY_MAPPINGS[i].byteIndex] & KEY_MAPPINGS[i].mask) {
			printf("%s ", KEY_MAPPINGS[i].name);
		}
	}
	printf("]\n");
}

uint32_t swap_endianness(uint32_t value) {
	return ((value & 0x000000FF) << 24) |
		((value & 0x0000FF00) << 8) |
		((value & 0x00FF0000) >> 8) |
		((value & 0xFF000000) >> 24);
}

void rotate_left(unsigned char* buf, size_t size) {
	if (size == 0) return;
	unsigned char first = buf[0];
	for (size_t i = 0; i < size - 1; i++) {
		buf[i] = buf[i + 1];
	}
	buf[size - 1] = first;
}

void get_midi_port(RtMidiOut* midiout) {
	unsigned int numPorts = midiout->getPortCount();

	// If no ports are available, inform the user and exit.
	if (numPorts == 0) {
		printf("No MIDI ports available, waiting...\n");
		while (numPorts == 0) {
			Sleep(1000);
			numPorts = midiout->getPortCount();
		}
	}

	// List available MIDI ports.
	printf("\nAvailable MIDI ports:\n");
	for (unsigned int i = 0; i < numPorts; i++) {
		printf("\t[%u] %s\n", i, midiout->getPortName(i).c_str());
	}

	// Prompt the user to choose a MIDI port.
	int chosenPort = -1;
	char inputBuffer[100];

	while (chosenPort < 0 || chosenPort >= (int)numPorts) {
		printf("\nEnter the number of the MIDI port you want to use: ");
		fgets(inputBuffer, sizeof(inputBuffer), stdin);

		// Try to parse the input as an integer.
		if (sscanf_s(inputBuffer, "%d", &chosenPort) != 1) {
			printf("Invalid input. Please enter a number.\n");
			chosenPort = -1;  // Reset chosen port.
		}
	}

	// Open the chosen MIDI port.
	midiout->openPort(chosenPort);
	printf("Opened %s\n", midiout->getPortName(chosenPort).c_str());
}

int main_loop(bool verbose)
{
	int res;
	unsigned char buf[8];
	wchar_t wstr[MAX_STR];
	hid_device* handle;

	printf("Looking for device...\n");
	// Initialize the hidapi library
	res = hid_init();

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(0x0507, 0x0010, NULL);
	while (handle == NULL) {
		Sleep(1000);
		handle = hid_open(0x0507, 0x0010, NULL);
	}

	// Read the Manufacturer String
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	wprintf(L"Manufacturer String: %s\n", wstr);

	// Read the Product String
	res = hid_get_product_string(handle, wstr, MAX_STR);
	wprintf(L"Product String: %s\n", wstr);

	// Read the Serial Number String
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	wprintf(L"Serial Number String: (%d) %s\n", wstr[0], wstr);

	// Read Indexed String 1
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	wprintf(L"Indexed String 1: %s\n", wstr);


	RtMidiOut* midiout = new RtMidiOut();
	get_midi_port(midiout);
	printf("Ready...\n");
	uint32_t prevState = 0;

	while (true) {
		res = hid_read_timeout(handle, buf, sizeof(buf), 1000);
		if (res < 0) {
			printf("Keyboard disconnected?\n");
			break;
		}
		rotate_left(buf, sizeof(buf));
		if (res < 0)
			break;
		if (res > 0) {
			if (verbose) printPressedKeys(buf);
			uint32_t state = *(reinterpret_cast<uint32_t*>(buf));
			state = swap_endianness(state);

			for (auto& mapping : NOTE_MAPPINGS) {
				uint32_t mask = mapping.first;
				uint8_t midiNote = mapping.second;
				uint8_t msg_type = mapping.second < 5 ? 0xB0 : 0x90;  // send control messages for buttons, notes for keys
				// Note-on event
				if ((state & mask) && !(prevState & mask)) {
					std::vector<uint8_t> message = { msg_type, midiNote, 64 };  // channel 1, note-on with velocity 64
					midiout->sendMessage(&message);
					//printf("Sent note %02x on\n", midiNote);
				}
				// Note-off event
				else if (mapping.second > 4 && !(state & mask) && (prevState & mask)) {
					std::vector<uint8_t> message = { 0x80, midiNote, 64 };  // channel 1, note-off with velocity 64
					midiout->sendMessage(&message);
					//printf("Sent note %02x off\n", midiNote);
				}
			}

			prevState = state;

		}
	}
	// Finalize the hidapi library
	res = hid_exit();

	midiout->closePort();
	delete midiout;

	return 0;
}

int main(int argc, char* argv[]) {
	bool verbose = false;

	// Loop through all command line arguments
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0) {
			verbose = true;
			break;
		}
		else {
			printf("Unknow argument: %s\n", argv[i]);
			return 1;
		}
	}
	while (true)
		main_loop(verbose);
}
