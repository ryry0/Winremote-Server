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
    tcp_socket = tcp_connection.acceptConnection();
    std::cout << "Connection accepted!\n";
  }

  //cout the keys
  while (key_received != 0x03) {
    tcp_connection.recieveData(tcp_socket, (char *) &key_received,
        sizeof(key_received));

    std::cout << std::hex << (int)key_received << std::endl;

    if (key_received == 0x7f) { //send a virtual keyboard code
      remote_user_input.ki.dwFlags = 0; 
      remote_user_input.ki.wVk = VK_BACK;
      remote_user_input.ki.wScan = 0; //hardware scan code
      SendInput(1, &remote_user_input, sizeof(INPUT)); //send a key press

      remote_user_input.ki.dwFlags = KEYEVENTF_KEYUP; 
      SendInput(1, &remote_user_input, sizeof(INPUT)); //send a key up signal
    }
    else { //send a unicode event
      remote_user_input.ki.dwFlags = KEYEVENTF_UNICODE; 
      remote_user_input.ki.wVk = 0;
      remote_user_input.ki.wScan = key_received; //hardware scan code
      SendInput(1, &remote_user_input, sizeof(INPUT)); //send a key press
    }

    //remote_user_input.ki.dwFlags = KEYEVENTF_KEYUP; //send a key release
    //SendInput(1, &remote_user_input, sizeof(INPUT));
  }

  return 0;
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
    case 0x7f:
      converted_key = VK_BACK;
      break;
    case 0x0a:
      converted_key = VK_RETURN;
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
}
