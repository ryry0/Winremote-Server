/*
 * Author: Ryan - David Reyes
 * Need delete, home, end, arrow keys
 */
#define WINVER 0x0500 //this allows sendinput to be defined in windows.h
#include <iostream>
#include <iomanip>
#include <cstring>
#include <inttypes.h>
#include <windows.h>
#include <tcpipwin32.h>

#define NUM_MOD 4
#define SHIFT_MOD   0x01
#define CONTROL_MOD 0x04
#define MENU_MOD 0x08
#define LWIN_MOD 0x40

struct settings_t {
  int port;
  int tcp_socket;
  bool enable_xtended_feat;
};

struct x_key_received_t {
  char modifier;
  uint16_t key;
  bool key_press;
};

const int DEFAULT_PORT = 25525;

char ConvertKey(const char key);
char XConvertKey(const uint16_t key);
char XTranslateModifier(char modifier);
void SendVirtualKeyTap(INPUT &input);
void SendVirtualKeyHold(INPUT &input);
void SendVirtualKeyUp(INPUT &input);
void SendVirtualKeyChord(char first_key, char second_key, INPUT &input);
void SendUnicodeCharacter(char key, INPUT &input);

int main(int argc, char ** argv) {
  INPUT remote_user_input;
  TCP tcp_connection;

  settings_t settings;
  settings.port = DEFAULT_PORT;
  settings.tcp_socket = 0;
  settings.enable_xtended_feat = false;
  x_key_received_t x_key_received;
  char MOD_KEYS[NUM_MOD] = {SHIFT_MOD, CONTROL_MOD, MENU_MOD, LWIN_MOD};

  char key_received = 0x00;

  //generic keyboard event
  remote_user_input.type = INPUT_KEYBOARD;
  remote_user_input.ki.time = 0;
  remote_user_input.ki.dwExtraInfo = 0;

  //parse the arguments
  if (argc > 1) {
    for (int i = 0; i < argc; i ++) {
      if (  (strcmp(argv[i], "-p") ==0) ||
          (strcmp(argv[i], "--port") ==0) ) {
        if (i+1 < argc)
          settings.port = atoi(argv[i+1]);
      } //end if

      else if ((strcmp(argv[i], "-x") ==0) ||
          (strcmp(argv[i], "--xtended_features") == 0)) {
        settings.enable_xtended_feat = true;
        std::cout << "Extended features will be used.\n";
      }
    } //end for
  } //end if
  std::cout << "Port " << settings.port << " will be used.\n";

  //listen and accept an incoming connection
  if (tcp_connection.listenToPort(settings.port)) {
    settings.tcp_socket = tcp_connection.acceptConnection(); std::cout << "Connection accepted!\n";
  }

  //use the extended features of the program
  if (settings.enable_xtended_feat) {
    while (1) {
      if (tcp_connection.receiveData(settings.tcp_socket,
            (char *) &x_key_received, sizeof(x_key_received))) {

        //if key press
        if (x_key_received.key_press) {
          //handle the modifier
          if (x_key_received.modifier != 0) {
            for (int i = 0; i < NUM_MOD; i++) {
              remote_user_input.ki.wVk = XTranslateModifier(x_key_received.modifier & MOD_KEYS[i]);
              SendVirtualKeyHold(remote_user_input);
            }
          }
          std::cout << "modifier " << std::hex << (int) x_key_received.modifier << "\n";

          remote_user_input.ki.wVk = XConvertKey(x_key_received.key);
          std::cout << "converted key " << std::hex << (uint8_t) XConvertKey(x_key_received.key) << "\n";

          SendVirtualKeyHold(remote_user_input);
          std::cout << "press " << std::hex << x_key_received.key << "\n";
        } //end if (x_key_received.key_press)

        //else if key_release
        else {
          //handle the modifier
          for (int i = 0; i < NUM_MOD; i++) {
            remote_user_input.ki.wVk = XTranslateModifier((x_key_received.modifier | MOD_KEYS[i]) & MOD_KEYS[i]);
            SendVirtualKeyUp(remote_user_input);
          }

          //handle the key press
          remote_user_input.ki.wVk = XConvertKey(x_key_received.key);
          SendVirtualKeyUp(remote_user_input);
          std::cout << "release\n";
        } //end else
      } //end if (tcp_connection...
    } //end while
  } //if (settings.enable_xtended_feat)

  else { //do not use the extended features
    while (key_received != 0x04) {
      //cout the keys
      if (tcp_connection.receiveData(settings.tcp_socket, (char *) &key_received,
            sizeof(key_received))) {

        std::cout << std::hex << (int)key_received << std::endl;

        if (key_received == 0x08) { //ctrl-backspace
          SendVirtualKeyChord(VK_CONTROL, VK_BACK, remote_user_input);
        }
        else if (key_received == 0x06) { //ctrl f
          SendVirtualKeyChord(VK_CONTROL, 'F', remote_user_input);
        }
        else if (key_received == 0x10){ //ctrl p
          SendVirtualKeyChord(VK_CONTROL, 'P', remote_user_input);
        }
        else if (key_received == 0x0E){ //ctrl n
          SendVirtualKeyChord(VK_CONTROL, 'N', remote_user_input);
        }
        else if (key_received == 0x17) { //ctrl w
          SendVirtualKeyChord(VK_CONTROL, 'W', remote_user_input);
        }
        else if (key_received == 0x14) { //ctrl t
          SendVirtualKeyChord(VK_CONTROL, 'T', remote_user_input);
        }
        else if (key_received == 0x0c) { //ctrl l
          SendVirtualKeyChord(VK_CONTROL, 'L', remote_user_input);
        }
        else if (key_received == 0x18) { //ctrl x
          SendVirtualKeyChord(VK_CONTROL, 'X', remote_user_input);
        }
        else if (key_received == 0x16) { //ctrl v
          SendVirtualKeyChord(VK_CONTROL, 'V', remote_user_input);
        }
        else if (key_received == 0x15) { //ctrl u mapped to ctrl-z
          SendVirtualKeyChord(VK_CONTROL, 'Z', remote_user_input);
        }
        else if (key_received == 0x19) { //ctrl y mapped to ctrl-c
          SendVirtualKeyChord(VK_CONTROL, 'C', remote_user_input);
        }
        else if (isupper(key_received)) { //all uppercase characters
          SendVirtualKeyChord(VK_SHIFT, ConvertKey(key_received),
              remote_user_input); }
        else if (isdigit(key_received) || ispunct(key_received)) { //punctuation
          SendUnicodeCharacter(key_received, remote_user_input);
        }
        else {
          remote_user_input.ki.wVk = ConvertKey(key_received);
          SendVirtualKeyTap(remote_user_input);
        } //end else
      } //end if
      else
        break;
    } //end while
  } //end else { //do not use extended features

  return 0;
}

void SendUnicodeCharacter(char key, INPUT &input) {
  input.ki.dwFlags = KEYEVENTF_UNICODE;
  input.ki.wScan = key;
  input.ki.wVk = 0;
  input.ki.dwExtraInfo = 0;
  SendInput(1, &input, sizeof(INPUT)); //send a key press
}

void SendVirtualKeyChord(char first_key, char second_key, INPUT &input) {
  input.ki.wVk = first_key; //press them
  SendVirtualKeyHold(input);
  input.ki.wVk = second_key;
  SendVirtualKeyHold(input);

  input.ki.wVk = second_key; //release them
  SendVirtualKeyUp(input);
  input.ki.wVk = first_key;
  SendVirtualKeyUp(input);
}

void SendVirtualKeyTap(INPUT &input) {
  SendVirtualKeyHold(input);
  SendVirtualKeyUp(input);
}

void SendVirtualKeyHold(INPUT &input) {
  input.ki.dwFlags = 0;
  input.ki.wScan = 0; //hardware scan code
  SendInput(1, &input, sizeof(INPUT)); //send a key press
}

void SendVirtualKeyUp(INPUT &input) {
  input.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &input, sizeof(INPUT)); //send a key up signal
}

char ConvertKey(const char key) {
  char converted_key = 0x00;
  switch(key) {
    case ' ':
      converted_key = VK_SPACE;
      break;
    case 0x33:
      converted_key = VK_DELETE;
      break;
    case 0x09:
      converted_key = VK_TAB;
      break;
    case 0x7f:
      converted_key = VK_BACK;
      break;
    case 0x0a:
      converted_key = VK_RETURN;
      break;
    case 0x12:
      converted_key = VK_LWIN;
      break;
    case 0x1b:
      converted_key = VK_ESCAPE;
      break;
    default:
      if (isalpha(key))
        converted_key = toupper(key);
      else
        converted_key = 0x97;
      break;
  }
  return converted_key;
}


char XConvertKey(const uint16_t key) {
  char converted_key = '\0';
  switch(key) {
    case ' ':
      converted_key = VK_SPACE;
      break;
    case 0xffff:
      converted_key = VK_DELETE;
      break;
    case 0xff09:
      converted_key = VK_TAB;
      break;
    case 0xff08:
      converted_key = VK_BACK;
      break;
    case 0xff0d:
      converted_key = VK_RETURN;
      break;
    case 0x12:
      converted_key = VK_LWIN;
      break;
    case 0xff1b:
      converted_key = VK_ESCAPE;
      break;
    case 0xff51:
      converted_key = VK_LEFT;
      break;
    case 0xff52:
      converted_key = VK_UP;
      break;
    case 0xff54:
      converted_key = VK_DOWN;
      break;
    case 0xff53:
      converted_key = VK_RIGHT;
      break;
    case 0xff57:
      converted_key = VK_END;
      break;
    case 0xff56:
      converted_key = VK_NEXT;
      break;
    case 0xff55:
      converted_key = VK_PRIOR;
      break;
    case 0xff50:
      converted_key = VK_HOME;
      break;
    case 0xffe2: //check if it is one of the modifier keys
    case 0xffe3:
    case 0xffe4:
    case 0xffe9:
      converted_key = 0x97;
      break;
    case 0xffeb:
      converted_key = VK_LWIN;
      break;

    case ';':
    case ':':
      converted_key = VK_OEM_1;
      break;
    case '-':
    case '_':
      converted_key = VK_OEM_MINUS;
      break;
    case '+':
    case '=':
      converted_key = VK_OEM_PLUS;
      break;
    case '/':
    case '?':
      converted_key = VK_OEM_2;
      break;
    case '`':
    case '~':
      converted_key = VK_OEM_3;
      break;
    case '.':
    case '>':
      converted_key = VK_OEM_PERIOD;
      break;
    case ',':
    case '<':
      converted_key = VK_OEM_COMMA;
      break;
    case '[':
    case '{':
      converted_key = VK_OEM_4;
      break;
    case ']':
    case '}':
      converted_key = VK_OEM_6;
      break;
    case '\\':
    case '|':
      converted_key = VK_OEM_5;
      break;
    case '\'':
    case '"':
      converted_key = VK_OEM_7;
      break;

    default:
      //if (ispunct(key))
      //  converted_key = '\0';
      if (isalpha(key))
        converted_key = toupper(key);
      else if (isdigit(key))
        converted_key = key;
      else
        converted_key = 0x97;
      break;
  } //end switch

  return converted_key;
} //end char XConvertKey(const uint16_t key) {

char XTranslateModifier(char modifier) {
  char converted_key = 0;
  switch(modifier) {
    case SHIFT_MOD:
      converted_key = VK_SHIFT;
      break;
    case CONTROL_MOD:
      converted_key = VK_CONTROL;
      break;
    case MENU_MOD:
      converted_key = VK_MENU;
      break;
    case LWIN_MOD:
      converted_key = VK_LWIN;
      break;
  } //end switch

  return converted_key;
} //end char XTranslateModifier(char modifier) {
