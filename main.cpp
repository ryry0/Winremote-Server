/*
 * Author: Ryan - David Reyes
 * Need delete, home, end, arrow keys
 */
#define WINVER 0x0500 //this allows sendinput to be defined in windows.h
#include <iostream>
#include <iomanip>
#include <cstring>
#include <windows.h>
#include <tcpipwin32.h>

const int DEFAULT_PORT = 25525;

char ConvertKey(const char key);
void SendVirtualKeyTap(INPUT &input);
void SendVirtualKeyHold(INPUT &input);
void SendVirtualKeyUp(INPUT &input);
void SendVirtualKeyChord(char first_key, char second_key, INPUT &input);
void SendUnicodeCharacter(char key, INPUT &input);

int main(int argc, char ** argv) {
  INPUT remote_user_input;
  TCP tcp_connection;
  int port = DEFAULT_PORT,
      tcp_socket;
  char key_received = 0x00;

  //generic keyboard event
  remote_user_input.type = INPUT_KEYBOARD;
  remote_user_input.ki.time = 0;
  remote_user_input.ki.dwExtraInfo = 0;

  //parse the arguments
  if (argc > 1) {
    for (int i = 0; i < argc; i ++) {
      if (  (strcmp(argv[i], "-p")) ||
          (strcmp(argv[i], "--port") ) ) {
        if (i+1 < argc)
          port = atoi(argv[i+1]);
      } //end if
    } //end for
  } //end if
  else {
    std::cout << "Default port " << DEFAULT_PORT << " will be used.\n";
  }

  //listen and accept an incoming connection
  if (tcp_connection.listenToPort(port)) {
    tcp_socket = tcp_connection.acceptConnection(); std::cout << "Connection accepted!\n";
  }

  //cout the keys
  while (key_received != 0x04) {
    if (tcp_connection.receiveData(tcp_socket, (char *) &key_received,
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
