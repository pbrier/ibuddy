/*
 * ibuddy.cpp
 * send data to ibuddy HID device
 * Peter Brier (peter.brier@kekketek.nl)
 * Command line parameters: data string, e.g. "HRGBUD<>"
 *
 * Based on HIDAPI library. (http://github.com/signal11/hidapi/commits/master)
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"

#define HID_VID 0x1130
#define HID_PID 0x0001
#define HID_SIZE 9 // report size

// Headers needed for sleeping.
#ifdef _WIN32
  #include <windows.h>
  #define wait(t) Sleep(t)
#else
  #include <unistd.h>
  #define wait(t) usleep(1000*t)
#endif



hid_device *handle;
int report_size=HID_SIZE;

// Get the numeric value of an argument (0..255)
// Can be decimal, number or character
int get_val(char *s, int *d)
{
  if ( !strncmp(s, "0x", 2) ) // Hex 
    return sscanf(s, "%X", d);
  else if (s[0] == '@' ) // character
  {
    *d = (int)s[1];
    return 1;   
  }
  else
    return sscanf(s, "%d", d);
}


/**
*** send_code()
**/
void send_code(unsigned char c)
{
  unsigned char buf[128];
  int res=0;
  buf[0] = 0;
  buf[1] = 0x55;
  buf[2] = 0x53;
  buf[3] = 0x42;
  buf[4] = 0x43;
  buf[5] = 0x00;
  buf[6] = 0x40;
  buf[7] = 0x02;
  buf[8] = c; // code
  res = hid_write(handle, buf, report_size);
  if (res < 0) 
  {
    printf("Unable to write(): %d\n", report_size);
    printf("Error: %ls\n", hid_error(handle));
  }
}
  


/**
***
*** buddy_code()
***
*** Bit.......: 7 654 3 2 1 0
*** Function..: H BGR D U C A
***
**/
void buddy_code(char *s)
{
  unsigned char c = (15 << 4);
  int t;
  if ( sscanf(s, "%d", &t ) == 1 )
    wait(t);
  for(; *s; s++)
  {
    switch (*s)
    {
      case 'H': c ^= 1<<7; break; // heart
      case 'B': c ^= 1<<6; break; // Blue LED
      case 'G': c ^= 1<<5; break; // Green LED
      case 'R': c ^= 1<<4; break; // Red LED
      case 'D': c ^= 1<<3; break; // wing down
      case 'U': c ^= 1<<2; break; // wing up
      case 'C': c ^= 1<<1; break; // Clockwise
      case 'A': c ^= 1<<0; break; // Anti clockwise
    }
  }
  send_code(c);
}


  
/**
*** Main()
**/
int main(int argc, char* argv[])
{
  int res, verbose = 0, do_write=1, do_interactive = 0, do_reconnect = 0, vid = HID_VID, pid = HID_PID;
  unsigned char buf[256];
  #define MAX_STR 255
  wchar_t wstr[MAX_STR];

  struct hid_device_info *devs, *cur_dev;
  int i;

  #ifdef WIN32
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);
  #endif

  if ( argc == 1 )
  {
    printf("ibuddy: control ibuddy via USB HID communication\n" 
     "kekbot.org - rev 1.0 - " __DATE__ " " __TIME__ "\n"
     "USE: \n"
     "  hidcom[options] [data]\n"
     "  --vid=0xABCD      VID to use (default=0x%4.4X)\n"
     "  --pid=0xABCD      PID to use (default=0x%4.4X)\n"
     "  --verbose         Show more information\n"
     "  --size=65         Set the report size (default=%d bytes)\n"
     "  --nowrite         Only read, do not write data\n"
     "  --interactive     Read data from stdin and write to stdout\n",
     HID_VID, HID_PID, HID_SIZE
    );
    exit(1);
  }
  
  for(int i=1; i<argc; i++)
  {
    if ( !strncmp(argv[i], "--vid=", 6) ) sscanf(argv[i]+6, "%X",  &vid);
    if ( !strncmp(argv[i], "--pid=", 6) ) sscanf(argv[i]+6, "%X",  &pid);
    if ( !strncmp(argv[i], "--size=", 7) ) sscanf(argv[i]+7, "%d",  &report_size);
    if ( !strcmp(argv[i], "--verbose") ) verbose = 1;
    if ( !strcmp(argv[i], "--nowrite") ) do_write = 0;  
    if ( !strcmp(argv[i], "--reconnect") ) do_reconnect = 1;  
    if ( !strcmp(argv[i], "--interactive") ) do_interactive = 1;  
  }


  devs = hid_enumerate(HID_VID, HID_PID);
  cur_dev = devs;	
  while (cur_dev) 
  {
    if ( verbose ) 
    {
      printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
      printf("\n");
      printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
      printf("  Product:      %ls\n", cur_dev->product_string);
      printf("  Release:      %hx\n", cur_dev->release_number);
      printf("  Interface:    %d\n",  cur_dev->interface_number);
      printf("\n");
    }
    if ( cur_dev->interface_number == 1 ) 
      handle = hid_open_path(cur_dev->path);
    cur_dev = cur_dev->next;
  }
  hid_free_enumeration(devs);

  if (!handle) 
  {
    printf("unable to open device (0x%X, 0x%X)\n", vid, pid);
    return 1;
  }
  
  memset(buf, 0, sizeof(buf));
  if ( verbose ) 
  {
    // Read the Manufacturer String
    wstr[0] = 0x0000;
    res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
    if (res < 0)
      printf("Unable to read manufacturer string\n");
    else
      printf("Manufacturer String: %ls\n", wstr);
    // Read the Product String
    wstr[0] = 0x0000;
    res = hid_get_product_string(handle, wstr, MAX_STR);
    if (res < 0)
      printf("Unable to read product string\n");
    else
      printf("Product String: %ls\n", wstr);

    // Read the Serial Number String
    wstr[0] = 0x0000;
    res = hid_get_serial_number_string(handle, wstr, MAX_STR);
    if (res < 0)
      printf("Unable to read serial number string\n");
    else
      printf("Serial Number String: (%d) %ls", wstr[0], wstr);
    printf("\n");
  } 

  // send init string
  unsigned char init[] = { 0x00, 0x22, 0x09, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00 };
  res = hid_write(handle, init, sizeof(init));

  if ( do_write )
  {
    for(int i=0; i<argc; i++)
      buddy_code(argv[i]);
  }

  if ( do_interactive ) 
  {  
    while ( !feof(stdin) )
    {
      char str[MAX_STR];
      fgets(str , MAX_STR-1, stdin);
      if ( strlen(str) == 0 ) break;
      buddy_code(str);
    }
  }  
  hid_close(handle); 
  hid_exit();/* Free static HIDAPI objects. */
  
  return res;
}
